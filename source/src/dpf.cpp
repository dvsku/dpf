#include "dpf.hpp"

#include <thread>
#include <fstream>

using namespace dvsku::dpf;

///////////////////////////////////////////////////////////////////////////////
// INTERNAL

static dpf_result internal_create(std::vector<dpf_input_file> input_files, const dpf::FILE_PATH output_file, dpf_context* context);

static void internal_make_relative(dpf_input_file& input_file, const dpf::DIR_PATH& root);

///////////////////////////////////////////////////////////////////////////////
// PUBLIC

dpf_result dpf::create(std::vector<dpf_input_file>& input_files, const FILE_PATH& output_file, dpf_context* context) {
    return internal_create(input_files, output_file, context);
}

void dpf::create_async(std::vector<dpf_input_file>& input_files, const FILE_PATH& output_file, dpf_context* context) {
    std::thread t([input_files, output_file, context] {
        internal_create(input_files, output_file, context);
    });
    t.detach();
}

///////////////////////////////////////////////////////////////////////////////
// INTERNAL IMPL

dpf_result internal_create(std::vector<dpf_input_file> input_files, const dpf::FILE_PATH output_file, dpf_context* context) {
    dpf_result result;
    float      prog_change = 100.0f / input_files.size();
    size_t     file_count  = input_files.size();
    
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
    
    for (dpf_input_file& input_file : input_files) {
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

        if (context && context->base_path != "")
            internal_make_relative(input_file, context->base_path);

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

void internal_make_relative(dpf_input_file& input_file, const dpf::DIR_PATH& root) {
    input_file.path = std::filesystem::relative(input_file.path, root);
}
