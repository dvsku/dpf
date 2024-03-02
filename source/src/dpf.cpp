#include "dpf.hpp"

#include <thread>
#include <fstream>

using namespace dvsku::dpf;

///////////////////////////////////////////////////////////////////////////////
// INTERNAL

static dpf_result internal_create(dpf_inputs input_files, const dpf::FILE_PATH output_file, dpf_context* context);
static dpf_result internal_extract(const dpf::FILE_PATH input_file, const dpf::DIR_PATH output_dir, dpf_context* context);

static void internal_make_relative(dpf_input_file& input_file, const dpf::DIR_PATH& root);

///////////////////////////////////////////////////////////////////////////////
// PUBLIC

dpf_result dpf::create(dpf_inputs& input_files, const FILE_PATH& output_file, dpf_context* context) {
    return internal_create(input_files, output_file, context);
}

void dpf::create_async(dpf_inputs& input_files, const FILE_PATH& output_file, dpf_context* context) {
    std::thread t([input_files, output_file, context] {
        internal_create(input_files, output_file, context);
    });
    t.detach();
}

dpf_result dpf::extract(const FILE_PATH& input_file, const DIR_PATH& output_dir, dpf_context* context) {
    return internal_extract(input_file, output_dir, context);
}

void dpf::extract_async(const FILE_PATH& input_file, const DIR_PATH& output_dir, dpf_context* context) {
    std::thread t([input_file, output_dir, context] {
        internal_extract(input_file, output_dir, context);
    });
    t.detach();
}

///////////////////////////////////////////////////////////////////////////////
// INTERNAL IMPL

dpf_result internal_create(dpf_inputs input_files, const dpf::FILE_PATH output_file, dpf_context* context) {
    dpf_result result;
    size_t     file_count  = input_files.files.size();
    float      prog_change = 100.0f / file_count;
    
    if (context)
        context->invoke_start();

    std::ofstream fout;
    fout.open(output_file, std::ios::binary);

    if (!fout.is_open()) {
        if (context)
            context->invoke_error(result);

        return result;
    }

    fout.write("DPF ", 4);
    fout.write((char*)&file_count, sizeof(size_t));

    std::vector<char> buffer;
    
    for (dpf_input_file& input_file : input_files.files) {
        if (context && context->invoke_cancel()) {

            return result;
        }

        buffer.clear();

        if (input_file.op == dpf_op::add || input_file.op == dpf_op::modify) {
            std::ifstream fin(input_file.path, std::ios::binary);
            
            if (!fin.is_open()) {
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
                if (context)
                    context->invoke_error(result);

                return result;
            }
        }

        if (context)
            context->invoke_pre_process(input_file, buffer);

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

    if (context)
        context->invoke_finish(result);

    return result;
}

dpf_result internal_extract(const dpf::FILE_PATH input_file, const dpf::DIR_PATH output_dir, dpf_context* context) {
    dpf_result result;

    size_t file_count = 0U;

    std::ifstream fin;
    fin.open(input_file, std::ios::binary);

    if (!fin.is_open()) {
        if (context)
            context->invoke_error(result);

        return result;
    }

    char magic[4] = "";
    fin.read(magic, 4);

    if (magic[0] != 'D' || magic[1] != 'P' || magic[2] != 'F' || magic[3] != ' ') {
        if (context)
            context->invoke_error(result);

        return result;
    }

    fin.read((char*)&file_count, sizeof(size_t));

    float             prog_change = 100.0f / file_count;
    std::vector<char> buffer;
    size_t            u64         = 0U;

    for (size_t i = 0; i < file_count; i++) {
        std::string rel_path = "";
        dpf_op      op       = dpf_op::undefined;

        fin.read((char*)&op, sizeof(dpf_op));

        if (op == dpf_op::undefined) {
            if (context)
                context->invoke_error(result);

            return result;
        }

        fin.read((char*)&u64, sizeof(size_t));

        rel_path.resize(u64);
        fin.read(rel_path.data(), u64);

        if (op == dpf_op::add || op == dpf_op::modify) {
            fin.read((char*)&u64, sizeof(size_t));

            buffer.resize(u64);
            buffer.clear();
            
            fin.read(buffer.data(), u64);

            std::filesystem::path filename = std::filesystem::path(output_dir).append(rel_path);
            std::ofstream         fout(filename, std::ios::binary);

            if (!fout.is_open()) {
                if (context)
                    context->invoke_error(result);

                return result;
            }

            fout.write(buffer.data(), buffer.size());
            fout.close();
        }

        if (context)
            context->invoke_update(prog_change);
    }
    
    fin.close();

    if (context)
        context->invoke_finish(result);

    return result;
}

void internal_make_relative(dpf_input_file& input_file, const dpf::DIR_PATH& root) {
    input_file.path = std::filesystem::relative(input_file.path, root);
}
