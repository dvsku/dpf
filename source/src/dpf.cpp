#include "dpf.hpp"
#include "utilities/dpf_util_binr.hpp"

#include <thread>
#include <fstream>
#include <miniz\miniz.h>
#include <md5\md5.hpp>

using namespace dvsku::dpf;

///////////////////////////////////////////////////////////////////////////////
// INTERNAL

struct dpf_header {
    char   magic[4]     = { 0, 0, 0, 0 };
    char   checksum[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    size_t file_count   = 0U;
};

static dpf_result internal_create(dpf_inputs input_files, const dpf::FILE_PATH dpf_file, dpf_context* context);
static dpf_result internal_patch(const dpf::FILE_PATH dpf_file, const dpf::DIR_PATH patch_dir, dpf_context* context);

static dpf_result internal_read_header(dpf_util_binr& binr, dpf_header& header);
static void internal_make_relative(dpf_file_mod& file_mod, const dpf::DIR_PATH& root);
static bool internal_get_md5(const dpf::FILE_PATH dpf_file, unsigned char* md5);

///////////////////////////////////////////////////////////////////////////////
// PUBLIC

dpf_result dpf::create(dpf_inputs& input_files, const FILE_PATH& dpf_file, dpf_context* context) {
    try {
        return internal_create(input_files, dpf_file, context);
    }
    catch (const std::exception& e) {
        dpf_result result;
        result.status  = dpf_status::error;
        result.message = e.what();

        return result;
    }
    catch (...) {
        dpf_result result;
        result.status  = dpf_status::error;
        result.message = "Critical failure.";

        return result;
    }
}

void dpf::create_async(dpf_inputs& input_files, const FILE_PATH& dpf_file, dpf_context* context) {
    std::thread t([input_files, dpf_file, context] {
        try {
            internal_create(input_files, dpf_file, context);
        }
        catch (const std::exception& e) {
            if (context) {
                dpf_result result;
                result.status = dpf_status::error;
                result.message = e.what();

                context->invoke_error(result);
            }
        }
        catch (...) {
            if (context) {
                dpf_result result;
                result.status = dpf_status::error;
                result.message = "Critical failure.";

                context->invoke_error(result);
            }
        }
    });
    t.detach();
}

dpf_result dpf::patch(const FILE_PATH& dpf_file, const DIR_PATH& patch_dir, dpf_context* context) {
    try {
        return internal_patch(dpf_file, patch_dir, context);
    }
    catch (const std::exception& e) {
        dpf_result result;
        result.status = dpf_status::error;
        result.message = e.what();

        return result;
    }
    catch (...) {
        dpf_result result;
        result.status = dpf_status::error;
        result.message = "Critical failure.";

        return result;
    }
}

void dpf::patch_async(const FILE_PATH& dpf_file, const DIR_PATH& patch_dir, dpf_context* context) {
    std::thread t([dpf_file, patch_dir, context] {
        try {
            internal_patch(dpf_file, patch_dir, context);
        }
        catch (const std::exception& e) {
            if (context) {
                dpf_result result;
                result.status  = dpf_status::error;
                result.message = e.what();

                context->invoke_error(result);
            }    
        }
        catch (...) {
            if (context) {
                dpf_result result;
                result.status  = dpf_status::error;
                result.message = "Critical failure.";

                context->invoke_error(result);
            }
        }
    });
    t.detach();
}

dpf_result dpf::get_files(const FILE_PATH& dpf_file, std::vector<std::string>& files) {
    dpf_result result;
    size_t     file_count = 0U;

    std::ifstream fin;
    fin.open(dpf_file, std::ios::binary);

    if (!fin.is_open()) {
        result.status  = dpf_status::error;
        result.message = "Failed to open `" + dpf_file.string() + "` file.";
        return result;
    }

    char magic[4] = "";
    fin.read(magic, 4);

    if (magic[0] != 'D' || magic[1] != 'P' || magic[2] != 'F' || magic[3] != ' ') {
        result.status = dpf_status::error;
        result.message = "`" + dpf_file.string() + "` is not a dpf file.";
        return result;
    }

    fin.seekg(16, std::ios_base::cur);
    fin.read((char*)&file_count, sizeof(size_t));

    size_t u64 = 0U;

    for (size_t i = 0; i < file_count; i++) {
        std::string relative_file_path = "";
        dpf_op      op                 = dpf_op::undefined;

        // Read patch operation

        fin.read((char*)&op, sizeof(op));

        // Read relative file path

        fin.read((char*)&u64, sizeof(size_t));
        relative_file_path.resize(u64);
        fin.read(relative_file_path.data(), u64);

        // Skip file content

        if (op == dpf_op::add || op == dpf_op::modify) {
            fin.seekg(sizeof(size_t), std::ios_base::cur);
            fin.read((char*)&u64, sizeof(size_t));
            fin.seekg(u64, std::ios_base::cur);
        }

        files.push_back(relative_file_path);
    }

    result.status = dpf_status::finished;
    return result;
}

bool dpf::check_checksum(const FILE_PATH& dpf_file) {
    char md5[16]          = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    char computed_md5[16] = { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    char magic[4]         = { 0, 0, 0, 0 };

    std::ifstream fin;
    fin.open(dpf_file, std::ios::binary);

    if (!fin.is_open())
        return false;

    fin.seekg(0, std::ios::end);
    std::streamsize file_size = fin.tellg();
    fin.seekg(0, std::ios::beg);

    if (file_size <= 20)
        return true;

    fin.read(magic, sizeof(magic));
    fin.read(md5,   sizeof(md5));
    fin.close();

    if (magic[0] != 'D' || magic[1] != 'P' || magic[2] != 'F' || magic[3] != ' ')
        return false;

    if (!internal_get_md5(dpf_file, (unsigned char*)computed_md5))
        return false;

    for (int i = 0; i < 16; i++) {
        if (md5[i] != computed_md5[i])
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// INTERNAL IMPL

dpf_result internal_create(dpf_inputs input_files, const dpf::FILE_PATH dpf_file, dpf_context* context) {
    dpf_result result;
    size_t     file_count  = input_files.files.size();
    float      prog_change = 100.0f / file_count;
    char       md5[16]     = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    
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
    fout.write(md5, 16);
    fout.write((char*)&file_count, sizeof(size_t));

    std::vector<uint8_t> buffer;
    
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

            fin.read((char*)buffer.data(), file_size);

            if (fin.bad()) {
                result.status  = dpf_status::error;
                result.message = "Failed to read input file `" + input_file.path.string() + "`.";

                if (context)
                    context->invoke_error(result);

                return result;
            }
        }

        if (context) {
            dpf_result process_result = context->invoke_buf_process(input_file, buffer);
            
            if (process_result.status != dpf_status::finished) {
                result.status  = dpf_status::error;
                result.message = "Failed to process buffer of `" + input_file.path.string() + "`. | " + process_result.message;

                if (context)
                    context->invoke_error(result);

                return result;
            }
        }

        if (input_files.base_path != "")
            internal_make_relative(input_file, input_files.base_path);

        std::string path = input_file.path.string();
        size_t      u64  = path.size();

        fout.write((char*)&input_file.op, sizeof(input_file.op));
        fout.write((char*)&u64, sizeof(size_t));
        fout.write(path.data(), u64);
        
        if (input_file.op == dpf_op::add || input_file.op == dpf_op::modify) {
            mz_ulong             max_compressed_size = mz_compressBound(static_cast<mz_ulong>(buffer.size()));
            mz_ulong             compressed_size     = max_compressed_size;
            std::vector<uint8_t> buffer_compress(max_compressed_size);

            int code = mz_compress(buffer_compress.data(), &compressed_size, buffer.data(), 
                static_cast<mz_ulong>(buffer.size()));

            if (code != MZ_OK) {
                result.status  = dpf_status::error;
                result.message = "Failed to compress input file `" + input_file.path.string() + "`.";

                if (context)
                    context->invoke_error(result);

                return result;
            }

            buffer_compress.resize(static_cast<size_t>(compressed_size));

            // Write decompressed size

            u64 = buffer.size();
            fout.write((char*)&u64, sizeof(u64));

            // Write compressed size

            u64 = buffer_compress.size();
            fout.write((char*)&u64, sizeof(u64));

            // Write content

            fout.write((char*)buffer_compress.data(), u64);
        }

        if (context)
            context->invoke_update(prog_change);
    }

    fout.close();

    // Create checksum

    internal_get_md5(dpf_file, (unsigned char*)md5);

    // Write checksum

    std::fstream fstream; 
    fstream.open(dpf_file, std::ios::binary | std::ios::in | std::ios::out);

    if (!fstream.is_open()) {
        result.status  = dpf_status::error;
        result.message = "Failed to open `" + dpf_file.string() + "` file.";

        if (context)
            context->invoke_error(result);

        return result;
    }

    fstream.seekp(4, std::ios::beg);
    fstream.write(md5, 16);
    fstream.close();

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

    fin.seekg(16, std::ios_base::cur);
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
            fin.seekg(sizeof(size_t), std::ios_base::cur);
            fin.read((char*)&u64, sizeof(size_t));
            fin.seekg(u64, std::ios_base::cur);
        }
        
        // Update progress

        if (context)
            context->invoke_update(prog_change_x);
    }

    fin.seekg(4 + sizeof(size_t) + 16, std::ios_base::beg);

    std::vector<uint8_t> compressed_buffer;
    std::vector<uint8_t> decompressed_buffer;

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
            size_t decompressed_size = 0U;
            size_t compressed_size   = 0U;

            fin.read((char*)&decompressed_size, sizeof(decompressed_size));
            fin.read((char*)&compressed_size,   sizeof(compressed_size));

            mz_ulong real_decompressed_size = static_cast<mz_ulong>(decompressed_size);

            compressed_buffer.resize(compressed_size);
            fin.read((char*)compressed_buffer.data(), compressed_size);

            std::filesystem::create_directories(filedir);

            std::ofstream fout(filename, std::ios::binary);
            if (!fout.is_open()) {
                result.status  = dpf_status::error;
                result.message = "Failed to open `" + filename.string() + "`.";

                if (context)
                    context->invoke_error(result);

                return result;
            }

            decompressed_buffer.resize(decompressed_size);

            int code = mz_uncompress(decompressed_buffer.data(), &real_decompressed_size, compressed_buffer.data(), 
                static_cast<mz_ulong>(compressed_size));

            if (code != MZ_OK || decompressed_size != real_decompressed_size) {
                result.status  = dpf_status::error;
                result.message = "Failed to decompress `" + filename.string() + "`.";

                if (context)
                    context->invoke_error(result);

                return result;
            }

            if (context) {
                dpf_file_mod file_mod;
                file_mod.path = filename;
                file_mod.op   = op;

                dpf_result process_result = context->invoke_buf_process(file_mod, decompressed_buffer);

                if (process_result.status != dpf_status::finished) {
                    result.status  = dpf_status::error;
                    result.message = "Failed to process buffer of `" + filename.string() + "`. | " + process_result.message;

                    if (context)
                        context->invoke_error(result);

                    return result;
                }
            }

            fout.write((char*)decompressed_buffer.data(), decompressed_buffer.size());
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

dpf_result internal_read_header(dpf_util_binr& binr, dpf_header& header) {
    dpf_result result;
    result.status = dpf_status::error;
    
    if (binr.size() < sizeof(header)) {
        result.message = "Header size mismatch.";
        return result;
    }

    binr.read_bytes(header.magic, sizeof(header.magic));

    if (header.magic[0] != 'D' || header.magic[1] != 'P' || header.magic[2] != 'F' || header.magic[3] != ' ') {
        result.message = "Not a DPF file.";
        return result;
    }

    binr.read_bytes(header.checksum, sizeof(header.checksum));
    header.file_count = binr.read_num<size_t>();

    result.status = dpf_status::finished;
    return result;
}

void internal_make_relative(dpf_file_mod& file_mod, const dpf::DIR_PATH& root) {
    file_mod.path = std::filesystem::relative(file_mod.path, root);
}

bool internal_get_md5(const dpf::FILE_PATH dpf_file, unsigned char* md5) {
    std::ifstream fin;
    fin.open(dpf_file, std::ios::binary);

    if (!fin.is_open())
        return false;

    fin.seekg(0, std::ios::end);
    std::streamsize file_size = fin.tellg();
    fin.seekg(0, std::ios::beg);

    if (file_size <= 20)
        return false;

    file_size -= 20;
    fin.seekg(20, std::ios::beg);

    std::vector<char> buffer;
    buffer.resize(file_size);

    fin.read(buffer.data(), file_size);

    MD5 md5_digest;
    md5_digest.add(buffer.data(), buffer.size());
    md5_digest.getHash(md5);

    return true;
}
