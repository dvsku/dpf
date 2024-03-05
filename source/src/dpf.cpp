#include "dpf.hpp"

#include <thread>
#include <fstream>

using namespace dvsku::dpf;

///////////////////////////////////////////////////////////////////////////////
// INTERNAL

static dpf_result internal_create(dpf_inputs input_files, const dpf::FILE_PATH dpf_file, dpf_context* context);
static dpf_result internal_patch(const dpf::FILE_PATH dpf_file, const dpf::DIR_PATH patch_dir, dpf_context* context);

static void internal_make_relative(dpf_file_mod& file_mod, const dpf::DIR_PATH& root);

///////////////////////////////////////////////////////////////////////////////
// PUBLIC

dpf_result dpf::create(dpf_inputs& input_files, const FILE_PATH& dpf_file, dpf_context* context) {
    return internal_create(input_files, dpf_file, context);
}

void dpf::create_async(dpf_inputs& input_files, const FILE_PATH& dpf_file, dpf_context* context) {
    std::thread t([input_files, dpf_file, context] {
        internal_create(input_files, dpf_file, context);
    });
    t.detach();
}

dpf_result dpf::patch(const FILE_PATH& dpf_file, const DIR_PATH& patch_dir, dpf_context* context) {
    return internal_patch(dpf_file, patch_dir, context);
}

void dpf::patch_async(const FILE_PATH& dpf_file, const DIR_PATH& patch_dir, dpf_context* context) {
    std::thread t([dpf_file, patch_dir, context] {
        internal_patch(dpf_file, patch_dir, context);
    });
    t.detach();
}

///////////////////////////////////////////////////////////////////////////////
// INTERNAL IMPL

dpf_result internal_create(dpf_inputs input_files, const dpf::FILE_PATH dpf_file, dpf_context* context) {
    dpf_result result;
    size_t     file_count  = input_files.files.size();
    float      prog_change = 100.0f / file_count;
    
    if (context)
        context->invoke_start();

    std::ofstream fout;
    fout.open(dpf_file, std::ios::binary);

    if (!fout.is_open()) {
        result.status  = dpf_status::error;
        result.message = "Failed to open `" + dpf_file.string() + "` file.";
        
        if (context)
            context->invoke_error(result);

        return result;
    }

    fout.write("DPF ", 4);
    fout.write((char*)&file_count, sizeof(size_t));

    std::vector<char> buffer;
    
    for (dpf_file_mod& input_file : input_files.files) {
        if (context && context->invoke_cancel()) {
            result.status = dpf_status::cancelled;
            return result;
        }

        buffer.clear();

        if (input_file.op == dpf_op::add || input_file.op == dpf_op::modify) {
            std::ifstream fin(input_file.path, std::ios::binary);
            
            if (!fin.is_open()) {
                result.status  = dpf_status::error;
                result.message = "Failed to open input file `" + input_file.path.string() + "`.";

                if (context)
                    context->invoke_error(result);

                return result;
            }

            fin.seekg(0, std::ios::end);
            std::streamsize file_size = fin.tellg();
            fin.seekg(0, std::ios::beg);

            buffer.resize(file_size);

            fin.read(buffer.data(), file_size);

            if (fin.bad()) {
                result.status  = dpf_status::error;
                result.message = "Failed to read input file `" + input_file.path.string() + "`.";

                if (context)
                    context->invoke_error(result);

                return result;
            }
        }

        if (context)
            context->invoke_buf_process(input_file, buffer);

        if (input_files.base_path != "")
            internal_make_relative(input_file, input_files.base_path);

        std::string path = input_file.path.string();
        size_t      u64  = path.size();

        fout.write((char*)&input_file.op, sizeof(input_file.op));
        fout.write((char*)&u64, sizeof(size_t));
        fout.write(path.data(), u64);
        
        if (input_file.op == dpf_op::add || input_file.op == dpf_op::modify) {
            u64 = buffer.size();

            fout.write((char*)&u64, sizeof(size_t));
            fout.write(buffer.data(), u64);
        }

        if (context)
            context->invoke_update(prog_change);
    }

    fout.close();

    result.status = dpf_status::finished;
    if (context)
        context->invoke_finish(result);

    return result;
}

dpf_result internal_patch(const dpf::FILE_PATH dpf_file, const dpf::DIR_PATH patch_dir, dpf_context* context) {
    dpf_result result;

    size_t file_count = 0U;

    std::ifstream fin;
    fin.open(dpf_file, std::ios::binary);

    if (!fin.is_open()) {
        result.status  = dpf_status::error;
        result.message = "Failed to open `" + dpf_file.string() + "` file.";

        if (context)
            context->invoke_error(result);

        return result;
    }

    char magic[4] = "";
    fin.read(magic, 4);

    if (magic[0] != 'D' || magic[1] != 'P' || magic[2] != 'F' || magic[3] != ' ') {
        result.status  = dpf_status::error;
        result.message = "`" + dpf_file.string() + "` is not a dpf file.";

        if (context)
            context->invoke_error(result);

        return result;
    }

    fin.read((char*)&file_count, sizeof(size_t));

    float  prog_change_x = 15.0f / file_count;
    float  prog_change_y = 85.0f / file_count;
    size_t u64           = 0U;

    for (size_t i = 0; i < file_count; i++) {
        std::string relative_file_path = "";
        dpf_op      op                 = dpf_op::undefined;

        // Read patch operation

        fin.read((char*)&op, sizeof(op));

        if (op == dpf_op::undefined) {
            result.status  = dpf_status::error;
            result.message = "Unsupported patch operation.";

            if (context)
                context->invoke_error(result);

            return result;
        }

        // Read relative file path

        fin.read((char*)&u64, sizeof(size_t));
        relative_file_path.resize(u64);
        fin.read(relative_file_path.data(), u64);

        // Check if file exists

        std::filesystem::path filename = std::filesystem::path(patch_dir).append(relative_file_path);

        if (op == dpf_op::modify || op == dpf_op::remove) {
            if (!std::filesystem::exists(filename)) {
                result.status  = dpf_status::error;
                result.message = "`" + filename.string() + "` doesn't exist.";

                if (context)
                    context->invoke_error(result);

                return result;
            }
        }

        // Skip file content

        if (op == dpf_op::add || op == dpf_op::modify) {
            fin.read((char*)&u64, sizeof(size_t));
            fin.seekg(u64, std::ios_base::cur);
        }
        
        // Update progress

        if (context)
            context->invoke_update(prog_change_x);
    }

    fin.seekg(4 + sizeof(size_t), std::ios_base::beg);

    std::vector<char> buffer;

    for (size_t i = 0; i < file_count; i++) {
        std::string relative_file_path = "";
        dpf_op      op                 = dpf_op::undefined;

        // Read patch operation

        fin.read((char*)&op, sizeof(op));

        // Read relative file path

        fin.read((char*)&u64, sizeof(size_t));
        relative_file_path.resize(u64);
        fin.read(relative_file_path.data(), u64);

        // Preform patch operation

        std::filesystem::path filename = std::filesystem::path(patch_dir).append(relative_file_path);
        std::filesystem::path filedir  = std::filesystem::path(filename).remove_filename();

        if (op == dpf_op::add || op == dpf_op::modify) {
            fin.read((char*)&u64, sizeof(size_t));

            buffer.resize(u64);
            fin.read(buffer.data(), u64);

            std::filesystem::create_directories(filedir);

            std::ofstream fout(filename, std::ios::binary);
            if (!fout.is_open()) {
                result.status  = dpf_status::error;
                result.message = "Failed to open `" + filename.string() + "`.";

                if (context)
                    context->invoke_error(result);

                return result;
            }

            fout.write(buffer.data(), buffer.size());
            fout.close();
        }
        else if (op == dpf_op::remove) {
            if (!std::filesystem::remove(filename)) {
                result.status  = dpf_status::error;
                result.message = "Failed to remove `" + filename.string() + "`.";

                if (context)
                    context->invoke_error(result);

                return result;
            }
        }

        // Update progress

        if (context)
            context->invoke_update(prog_change_y);
    }

    fin.close();

    result.status = dpf_status::finished;
    if (context)
        context->invoke_finish(result);

    return result;
}

void internal_make_relative(dpf_file_mod& file_mod, const dpf::DIR_PATH& root) {
    file_mod.path = std::filesystem::relative(file_mod.path, root);
}
