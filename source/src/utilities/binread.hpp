#pragma once

#include "utilities/string.hpp"

#include <fstream>

namespace libdpf {
    class binread {
    public:
        binread()               = delete;
        binread(const binread&) = delete;
        binread(binread&&)      = default;

        binread(std::ifstream& stream) : m_stream(stream) {
            seek(0, std::ios_base::end);
            m_size = pos();
            seek(0, std::ios_base::beg);
        }

        binread& operator=(const binread&) = delete;
        binread& operator=(binread&&)      = default;

    public:
        size_t pos() {
            return static_cast<size_t>(m_stream.tellg());
        }

        size_t size() const {
            return m_size;
        }

        void seek(size_t offset, std::ios_base::seekdir dir = std::ios_base::cur) {
            m_stream.seekg(offset, dir);
            m_pos = pos();
        }

        template<typename T>
        T read_num() {
            assert_can_read(sizeof(T));

            T value{};
            m_stream.read((char*)&value, sizeof(T));
            m_pos = pos();

            return value;
        }

        void read_bytes(char* ptr, size_t size) {
            assert_can_read(size);

            m_stream.read(ptr, size);
            m_pos = pos();
        }

        std::string read_str(std::size_t len) {
            assert_can_read(len);

            std::string value(len, 0);
            m_stream.read(value.data(), len);
            m_pos = pos();

            return value;
        }

    private:
        std::ifstream& m_stream;
        size_t         m_size = 0U;
        size_t         m_pos  = 0U;

    private:
        void assert_can_read(std::size_t len) {
            auto pos = this->pos();

            if (pos + len > m_size)
                throw std::runtime_error(DPF_FORMAT("Tried to read outside file bounds. | Read offset: {:x} Read len: {}", pos, len));
        }

        void assert_can_seek(std::size_t offset) {
            auto pos = this->pos();

            if (pos + offset > m_size)
                throw std::runtime_error(DPF_FORMAT("Tried to seek outside file bounds. | Current offset: {:x} Seek offset: {}", pos, offset));
        }
    };
}