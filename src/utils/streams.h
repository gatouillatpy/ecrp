// The MIT License (MIT)
// Simplistic Binary Streams 0.9 (https://github.com/shaovoon/simplebinstream)
// Copyright (C) 2014 - 2016, by Wong Shao Voon (shaovoon@yahoo.com)
//
// http://opensource.org/licenses/MIT
//
// version 0.9.2   : Optimize _mem_istream constructor for const byte*
// version 0.9.3   : Optimize _mem_ostream vector insert
// version 0.9.4   : New _ptr_istream class
// version 0.9.5   : Add Endianness Swap with compile time check
// version 0.9.6   : Using C File APIs, instead of STL file streams
// version 0.9.7   : Add _memfile_istream
// version 0.9.8   : Fix GCC and Clang template errors
// version 0.9.9   : Fix bug of getting previous value when reading empty string

#ifndef binstream_h
#define binstream_h

#include <type_traits>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <stdint.h>
#include <cstdio>

#include "byte.h"

namespace ecrp {
	namespace io {

#ifdef _WIN32
		using NativeEndian = std::true_type;
#if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
		using LittleEndian = std::true_type;
		using BigEndian = std::false_type;
#else
		using LittleEndian = std::true_type;
		using BigEndian = std::false_type;
#endif
#else
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		using LittleEndian = std::true_type;
		using BigEndian = std::false_type;
#else
		using LittleEndian = std::true_type;
		using BigEndian = std::false_type;
#endif
#endif // _WIN32

        template<typename T>
        void swap_endian8(T &ui) {
            union EightBytes {
                T ui;
                uint8_t arr[8];
            };

            EightBytes fb;
            fb.ui = ui;
            // swap the endian
            std::swap(fb.arr[0], fb.arr[7]);
            std::swap(fb.arr[1], fb.arr[6]);
            std::swap(fb.arr[2], fb.arr[5]);
            std::swap(fb.arr[3], fb.arr[4]);

            ui = fb.ui;
        }

        template<typename T>
        void swap_endian4(T &ui) {
            union FourBytes {
                T ui;
                uint8_t arr[4];
            };

            FourBytes fb;
            fb.ui = ui;
            // swap the endian
            std::swap(fb.arr[0], fb.arr[3]);
            std::swap(fb.arr[1], fb.arr[2]);

            ui = fb.ui;
        }

        template<typename T>
        void swap_endian2(T &ui) {
            union TwoBytes {
                T ui;
                uint8_t arr[2];
            };

            TwoBytes fb;
            fb.ui = ui;
            // swap the endian
            std::swap(fb.arr[0], fb.arr[1]);

            ui = fb.ui;
        }

        template<typename T>
        void swap_if_integral(T &val, std::true_type) {
            switch(sizeof(T)) {
                case 2u: swap_endian2(val); break;
                case 4u: swap_endian4(val); break;
                case 8u: swap_endian8(val); break;
            }
        }

        template<typename T>
        void swap_if_integral(T &val, std::false_type) {
            // T is not integral so do nothing
        }

        template<typename T>
        void swap(T &val, std::false_type) {
            std::is_integral<T> is_integral_type;
            swap_if_integral(val, is_integral_type);
        }

        template<typename T>
        void swap(T &val, std::true_type) {
            // same endian so do nothing.
        }

        template<typename same_endian_type>
        class _file_istream {
            public:
                _file_istream() : input__file_ptr(nullptr), _file_size(0L), read_length(0L) {}
                _file_istream(const char *file) : input__file_ptr(nullptr), _file_size(0L), read_length(0L) {
                    open(file);
                }
                ~_file_istream() {
                    close();
                }
                void open(const char *file) {
                    close();
#ifdef _MSC_VER
                    input__file_ptr = nullptr;
                    fopen_s(&input__file_ptr, file, "rb");
#else
                    input__file_ptr = std::fopen(file, "rb");
#endif
                    compute_length();
                }
                void close() {
                    if (input__file_ptr) {
                        fclose(input__file_ptr);
                        input__file_ptr = nullptr;
                    }
                }
                bool is_open() {
                    return (input__file_ptr != nullptr);
                }
                long _file_length() const {
                    return _file_size;
                }
                // http://www.cplusplus.com/reference/cstdio/feof/
                // stream's internal position indicator may point to the end-of-file for the
                // next operation, but still, the end-of-file indicator may not be set until
                // an operation attempts to read at that point.
                bool eof() const { // not using feof(), see above
                    return read_length >= _file_size;
                }
                long tellg() const {
                    return std::ftell(input__file_ptr);
                }
                void seekg (long pos) {
                    std::fseek(input__file_ptr, pos, SEEK_SET);
                }
                void seekg (long offset, int way) {
                    std::fseek(input__file_ptr, offset, way);
                }

                template<typename T>
                void read(T &t) {
                    if (std::fread(reinterpret_cast<void *>(&t), sizeof(T), 1, input__file_ptr) != 1) {
                        throw std::runtime_error("Read Error!");
                    }
                    read_length += sizeof(T);
					ecrp::io::swap(t, m_same_type);
                }
                void read(typename std::vector<byte> &vec) {
                    if (std::fread(reinterpret_cast<void *>(&vec[0]), vec.size(), 1, input__file_ptr) != 1) {
                        throw std::runtime_error("Read Error!");
                    }
                    read_length += vec.size();
                }
                void read(byte *p, size_t size) {
                    if (std::fread(reinterpret_cast<void *>(p), size, 1, input__file_ptr) != 1) {
                        throw std::runtime_error("Read Error!");
                    }
                    read_length += size;
                }
            private:
                void compute_length() {
                    seekg(0, SEEK_END);
                    _file_size = tellg();
                    seekg(0, SEEK_SET);
                }

                std::FILE *input__file_ptr;
                long _file_size;
                long read_length;
                same_endian_type m_same_type;
        };

        template<typename same_endian_type, typename T>
        _file_istream<same_endian_type> &operator >> ( _file_istream<same_endian_type> &istm, T &val) {
            istm.read(val);

            return istm;
        }

        template<typename same_endian_type>
        _file_istream<same_endian_type> &operator >> ( _file_istream<same_endian_type> &istm, std::string &val) {
            val.clear();

            int size = 0;
            istm.read(size);

            if (size <= 0) {
                return istm;
            }

            std::vector<byte> vec((size_t)size);
            istm.read(vec);
            val.assign(&vec[0], (size_t)size);

            return istm;
        }

        template<typename same_endian_type>
        class _mem_istream {
            public:
                _mem_istream() : m_index(0) {}
                _mem_istream(const byte *mem, size_t size) {
                    open(mem, size);
                }
                _mem_istream(const std::vector<byte> &vec) {
                    m_index = 0;
                    m_vec.reserve(vec.size());
                    m_vec.assign(vec.begin(), vec.end());
                }
                void open(const byte *mem, size_t size) {
                    m_index = 0;
                    m_vec.clear();
                    m_vec.reserve(size);
                    m_vec.assign(mem, mem + size);
                }
                void close() {
                    m_vec.clear();
                }
                bool eof() const {
                    return m_index >= m_vec.size();
                }
                std::ifstream::pos_type tellg() {
                    return m_index;
                }
                bool seekg(size_t pos) {
                    if (pos < m_vec.size()) {
                        m_index = pos;
                    } else {
                        return false;
                    }

                    return true;
                }
                bool seekg(std::streamoff offset, std::ios_base::seekdir way) {
                    if (way == std::ios_base::beg && offset < m_vec.size()) {
                        m_index = offset;
                    } else if (way == std::ios_base::cur && (m_index + offset) < m_vec.size()) {
                        m_index += offset;
                    } else if (way == std::ios_base::end && (m_vec.size() + offset) < m_vec.size()) {
                        m_index = m_vec.size() + offset;
                    } else {
                        return false;
                    }

                    return true;
                }

                const std::vector<byte> &get_internal_vec() {
                    return m_vec;
                }

                template<typename T>
                void read(T &t) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + sizeof(T)) > m_vec.size()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(&t), &m_vec[m_index], sizeof(T));

                    ecrp::io::swap(t, m_same_type);

                    m_index += sizeof(T);
                }

                void read(typename std::vector<byte> &vec) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + vec.size()) > m_vec.size()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(&vec[0]), &m_vec[m_index], vec.size());

                    m_index += vec.size();
                }

                void read(byte *p, size_t size) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + size) > m_vec.size()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(p), &m_vec[m_index], size);

                    m_index += size;
                }

                void read(std::string &str, const unsigned int size) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + str.size()) > m_vec.size()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    str.assign(&m_vec[m_index], size);

                    m_index += str.size();
                }

            private:
                std::vector<byte> m_vec;
                size_t m_index;
                same_endian_type m_same_type;
        };

        template<typename same_endian_type, typename T>
        _mem_istream<same_endian_type> &operator >> ( _mem_istream<same_endian_type> &istm, T &val) {
            istm.read(val);

            return istm;
        }

        template<typename same_endian_type>
        _mem_istream<same_endian_type> &operator >> (_mem_istream<same_endian_type> &istm, std::string &val) {
            val.clear();

            int size = 0;
            istm.read(size);

            if (size <= 0) {
                return istm;
            }

            istm.read(val, size);

            return istm;
        }

        template<typename same_endian_type>
        class _ptr_istream {
            public:
                _ptr_istream() : m_arr(nullptr), m_size(0), m_index(0) {}
                _ptr_istream(const byte *mem, size_t size) : m_arr(nullptr), m_size(0), m_index(0) {
                    open(mem, size);
                }
                _ptr_istream(const std::vector<byte> &vec) {
                    m_index = 0;
                    m_arr = vec.data();
                    m_size = vec.size();
                }
                void open(const byte *mem, size_t size) {
                    m_index = 0;
                    m_arr = mem;
                    m_size = size;
                }
                void close() {
                    m_arr = nullptr; m_size = 0; m_index = 0;
                }
                bool eof() const {
                    return m_index >= m_size;
                }
                std::ifstream::pos_type tellg() {
                    return m_index;
                }
                bool seekg(size_t pos) {
                    if (pos < m_size) {
                        m_index = pos;
                    } else {
                        return false;
                    }

                    return true;
                }
                bool seekg(std::streamoff offset, std::ios_base::seekdir way) {
                    if (way == std::ios_base::beg && offset < m_size) {
                        m_index = offset;
                    } else if (way == std::ios_base::cur && (m_index + offset) < m_size) {
                        m_index += offset;
                    } else if (way == std::ios_base::end && (m_size + offset) < m_size) {
                        m_index = m_size + offset;
                    } else {
                        return false;
                    }

                    return true;
                }

                template<typename T>
                void read(T &t) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + sizeof(T)) > m_size) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(&t), &m_arr[m_index], sizeof(T));

					ecrp::io::swap(t, m_same_type);

                    m_index += sizeof(T);
                }

                void read(typename std::vector<byte> &vec) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + vec.size()) > m_size) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(&vec[0]), &m_arr[m_index], vec.size());

                    m_index += vec.size();
                }

                void read(byte *p, size_t size) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + size) > m_size) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(p), &m_arr[m_index], size);

                    m_index += size;
                }

                void read(std::string &str, const unsigned int size) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + str.size()) > m_size) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    str.assign(&m_arr[m_index], size);

                    m_index += str.size();
                }

            private:
                const byte *m_arr;
                size_t m_size;
                size_t m_index;
                same_endian_type m_same_type;
        };


        template<typename same_endian_type, typename T>
        _ptr_istream<same_endian_type> &operator >> ( _ptr_istream<same_endian_type> &istm, T &val) {
            istm.read(val);

            return istm;
        }

        template<typename same_endian_type>
        _ptr_istream<same_endian_type> &operator >> ( _ptr_istream<same_endian_type> &istm, std::string &val) {
            val.clear();

            int size = 0;
            istm.read(size);

            if (size <= 0) {
                return istm;
            }

            istm.read(val, size);

            return istm;
        }

        template<typename same_endian_type>
        class _memfile_istream {
            public:
                _memfile_istream() : m_arr(nullptr), m_size(0), m_index(0) {}
                _memfile_istream(const char *file) : m_arr(nullptr), m_size(0), m_index(0) {
                    open(file);
                }
                ~_memfile_istream() {
                    close();
                }
                void open(const char *file) {
                    close();
#ifdef _MSC_VER
                    std::FILE *input__file_ptr = nullptr;
                    fopen_s(&input__file_ptr, file, "rb");
#else
                    std::FILE *input__file_ptr = std::fopen(file, "rb");
#endif
                    compute_length(input__file_ptr);
                    m_arr = new byte[m_size];
                    std::fread(m_arr, m_size, 1, input__file_ptr);
                    fclose(input__file_ptr);
                }
                void close() {
                    if (m_arr) {
                        delete[] m_arr;
                        m_arr = nullptr; m_size = 0; m_index = 0;
                    }
                }
                bool is_open() {
                    return (m_arr != nullptr);
                }
                long _file_length() const {
                    return m_size;
                }
                bool eof() const {
                    return m_index >= m_size;
                }
                long tellg(std::FILE *input__file_ptr) const {
                    return std::ftell(input__file_ptr);
                }
                void seekg(std::FILE *input__file_ptr, long pos) {
                    std::fseek(input__file_ptr, pos, SEEK_SET);
                }
                void seekg(std::FILE *input__file_ptr, long offset, int way) {
                    std::fseek(input__file_ptr, offset, way);
                }

                template<typename T>
                void read(T &t) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + sizeof(T)) > m_size) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(&t), &m_arr[m_index], sizeof(T));

					ecrp::io::swap(t, m_same_type);

                    m_index += sizeof(T);
                }

                void read(typename std::vector<byte> &vec) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + vec.size()) > m_size) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(&vec[0]), &m_arr[m_index], vec.size());

                    m_index += vec.size();
                }

                void read(byte *p, size_t size) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + size) > m_size) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    std::memcpy(reinterpret_cast<void *>(p), &m_arr[m_index], size);

                    m_index += size;
                }

                void read(std::string &str, const unsigned int size) {
                    if (eof()) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    if ((m_index + str.size()) > m_size) {
                        throw std::runtime_error("Premature end of array!");
                    }

                    str.assign(&m_arr[m_index], size);

                    m_index += str.size();
                }

            private:
                void compute_length(std::FILE *input__file_ptr) {
                    seekg(input__file_ptr, 0, SEEK_END);
                    m_size = tellg(input__file_ptr);
                    seekg(input__file_ptr, 0, SEEK_SET);
                }

                byte *m_arr;
                size_t m_size;
                size_t m_index;
                same_endian_type m_same_type;
        };


        template<typename same_endian_type, typename T>
        _memfile_istream<same_endian_type> &operator >> ( _memfile_istream<same_endian_type> &istm, T &val) {
            istm.read(val);

            return istm;
        }

        template<typename same_endian_type>
        _memfile_istream<same_endian_type> &operator >> ( _memfile_istream<same_endian_type> &istm, std::string &val) {
            val.clear();

            int size = 0;
            istm.read(size);

            if (size <= 0) {
                return istm;
            }

            istm.read(val, size);

            return istm;
        }

        template<typename same_endian_type>
        class _file_ostream {
            public:
                _file_ostream() : output__file_ptr(nullptr) {}
                _file_ostream(const char *file) : output__file_ptr(nullptr) {
                    open(file);
                }
                ~_file_ostream() {
                    close();
                }
                void open(const char *file) {
                    close();
#ifdef _MSC_VER
                    output__file_ptr = nullptr;
                    fopen_s(&output__file_ptr, file, "wb");
#else
                    output__file_ptr = std::fopen(file, "wb");
#endif
                }
                void flush() {
                    std::fflush(output__file_ptr);
                }
                void close() {
                    if (output__file_ptr) {
                        std::fclose(output__file_ptr);
                        output__file_ptr = nullptr;
                    }
                }
                bool is_open() {
                    return output__file_ptr != nullptr;
                }
                template<typename T>
                void write(const T &t) {
                    T t2 = t;
					ecrp::io::swap(t2, m_same_type);
                    std::fwrite(reinterpret_cast<const void *>(&t2), sizeof(T), 1, output__file_ptr);
                }
                void write(const std::vector<byte> &vec) {
                    std::fwrite(reinterpret_cast<const void *>(&vec[0]), vec.size(), 1, output__file_ptr);
                }
                void write(const byte *p, size_t size) {
                    std::fwrite(reinterpret_cast<const void *>(p), size, 1, output__file_ptr);
                }

            private:
                std::FILE *output__file_ptr;
                same_endian_type m_same_type;
        };

        template<typename same_endian_type, typename T>
        _file_ostream<same_endian_type> &operator << (_file_ostream<same_endian_type> &ostm, const T &val) {
            ostm.write(val);

            return ostm;
        }

        template<typename same_endian_type>
        _file_ostream<same_endian_type> &operator << ( _file_ostream<same_endian_type> &ostm, const std::string &val) {
            int size = val.size();
            ostm.write(size);

            if (val.size() <= 0) {
                return ostm;
            }

            ostm.write(val.c_str(), val.size());

            return ostm;
        }

        template<typename same_endian_type>
        _file_ostream<same_endian_type> &operator << ( _file_ostream<same_endian_type> &ostm, const byte *val) {
            int size = std::strlen(val);
            ostm.write(size);

            if (size <= 0) {
                return ostm;
            }

            ostm.write(val, size);

            return ostm;
        }

        template<typename same_endian_type>
        class _mem_ostream {
            public:
                _mem_ostream() {}
                void close() {
                    m_vec.clear();
                }
                const std::vector<byte> &get_internal_vec() {
                    return m_vec;
                }
                template<typename T>
                void write(const T &t) {
                    std::vector<byte> vec(sizeof(T));
                    T t2 = t;
					ecrp::io::swap(t2, m_same_type);
                    std::memcpy(reinterpret_cast<void *>(&vec[0]), reinterpret_cast<const void *>(&t2), sizeof(T));
                    write(vec);
                }
                void write(const std::vector<byte> &vec) {
                    m_vec.insert(m_vec.end(), vec.begin(), vec.end());
                }
                void write(const byte *p, size_t size) {
                    for(size_t i = 0; i < size; ++i) {
                        m_vec.push_back(p[i]);
                    }
                }

            private:
                std::vector<byte> m_vec;
                same_endian_type m_same_type;
        };

        template<typename same_endian_type, typename T>
        _mem_ostream<same_endian_type> &operator << ( _mem_ostream<same_endian_type> &ostm, const T &val) {
            ostm.write(val);

            return ostm;
        }

        template<typename same_endian_type>
        _mem_ostream<same_endian_type> &operator << ( _mem_ostream<same_endian_type> &ostm, const std::string &val) {
            int size = val.size();
            ostm.write(size);

            if (val.size() <= 0) {
                return ostm;
            }

            ostm.write(val.c_str(), val.size());

            return ostm;
        }

        template<typename same_endian_type>
        _mem_ostream<same_endian_type> &operator << ( _mem_ostream<same_endian_type> &ostm, const byte *val) {
            int size = std::strlen(val);
            ostm.write(size);

            if (size <= 0) {
                return ostm;
            }

            ostm.write(val, size);

            return ostm;
        }

        template<typename same_endian_type>
        class _memfile_ostream {
            public:
                _memfile_ostream() {}
                void close() {
                    m_vec.clear();
                }
                const std::vector<byte> &get_internal_vec() {
                    return m_vec;
                }
                template<typename T>
                void write(const T &t) {
                    std::vector<byte> vec(sizeof(T));
                    T t2 = t;
					ecrp::io::swap(t2, m_same_type);
                    std::memcpy(reinterpret_cast<void *>(&vec[0]), reinterpret_cast<const void *>(&t2), sizeof(T));
                    write(vec);
                }
                void write(const std::vector<byte> &vec) {
                    m_vec.insert(m_vec.end(), vec.begin(), vec.end());
                }
                void write(const byte *p, size_t size) {
                    for (size_t i = 0; i < size; ++i) {
                        m_vec.push_back(p[i]);
                    }
                }
                bool write_to_file(const char *file) {
#ifdef _MSC_VER
                    std::FILE *fp = nullptr;
                    fopen_s(&fp, file, "wb");
#else
                    std::FILE *fp = std::fopen(file, "wb");
#endif
                    if (fp) {
                        size_t size = std::fwrite(m_vec.data(), m_vec.size(), 1, fp);
                        std::fflush(fp);
                        std::fclose(fp);
                        m_vec.clear();
                        return size == 1u;
                    }
                    return false;
                }

            private:
                std::vector<byte> m_vec;
                same_endian_type m_same_type;
        };

        template<typename same_endian_type, typename T>
        _memfile_ostream<same_endian_type> &operator << (_memfile_ostream<same_endian_type> &ostm, const T &val) {
            ostm.write(val);

            return ostm;
        }

        template<typename same_endian_type>
        _memfile_ostream<same_endian_type> &operator << (_memfile_ostream<same_endian_type> &ostm, const std::string &val) {
            int size = val.size();
            ostm.write(size);

            if (val.size() <= 0) {
                return ostm;
            }

            ostm.write(val.c_str(), val.size());

            return ostm;
        }

        template<typename same_endian_type>
        _memfile_ostream<same_endian_type> &operator << (_memfile_ostream<same_endian_type> &ostm, const byte *val) {
            int size = std::strlen(val);
            ostm.write(size);

            if (size <= 0) {
                return ostm;
            }

            ostm.write(val, size);

            return ostm;
        }

		typedef _file_istream<NativeEndian> file_istream;
		typedef _file_istream<LittleEndian> le_file_istream;
		typedef _file_istream<BigEndian> be_file_istream;

		typedef _mem_istream<NativeEndian> mem_istream;
		typedef _mem_istream<LittleEndian> le_mem_istream;
		typedef _mem_istream<BigEndian> be_mem_istream;

		typedef _ptr_istream<NativeEndian> ptr_istream;
		typedef _ptr_istream<LittleEndian> le_ptr_istream;
		typedef _ptr_istream<BigEndian> be_ptr_istream;

		typedef _memfile_istream<NativeEndian> memfile_istream;
		typedef _memfile_istream<LittleEndian> le_memfile_istream;
		typedef _memfile_istream<BigEndian> be_memfile_istream;

		typedef _file_ostream<NativeEndian> file_ostream;
		typedef _file_ostream<LittleEndian> le_file_ostream;
		typedef _file_ostream<BigEndian> be_file_ostream;

		typedef _mem_ostream<NativeEndian> mem_ostream;
		typedef _mem_ostream<LittleEndian> le_mem_ostream;
		typedef _mem_ostream<BigEndian> be_mem_ostream;
	}
}

#endif // binstream_h