/**
 * \file
 * \brief A simple circular-buffer implementation.
 */
#pragma once

/* toolchain */
#include <array>
#include <cassert>
#include <cstring>

/* internal */
#include "../generated/ifgen/common.h"
#include "../generated/structs/BufferState.h"

namespace Coral
{

template <std::size_t depth, typename element_t = std::byte>
class CircularBuffer
{
    static_assert(depth > 0);

  public:
    CircularBuffer() : buffer(), state()
    {
    }

    inline std::size_t write_index(void)
    {
        return state.write_cursor % depth;
    }

    template <byte_size T, std::endian endianness = std::endian::native>
    inline void write(T elem)
        requires byte_size<element_t>
    {
        (void)endianness;
        write_single(static_cast<element_t>(elem));
    }

    template <typename T, std::endian endianness = std::endian::native>
    inline void write(T elem)
        requires byte_size<element_t> && (not byte_size<T>)
    {
        elem = handle_endian<T, endianness>(elem);
        write_n(reinterpret_cast<const element_t *>(&elem), sizeof(T));
    }

    template <ifgen_struct T, std::endian endianness = std::endian::native>
    inline void write(const T *elem)
        requires byte_size<element_t>
    {
        T temp = T();
        temp.template decode<endianness>(elem->raw_ro());
        write_n(reinterpret_cast<const element_t *>(temp.raw_ro()), T::size);
    }

    template <byte_size T, std::endian endianness = std::endian::native>
    inline T read(void)
        requires byte_size<element_t>
    {
        (void)endianness;
        return static_cast<T>(read_single());
    }

    template <typename T, std::endian endianness = std::endian::native>
    inline T read(void)
        requires byte_size<element_t> && (not byte_size<T>)
    {
        T result = T();
        read_n(reinterpret_cast<element_t *>(&result), sizeof(T));
        return handle_endian<T, endianness>(result);
    }

    template <ifgen_struct T, std::endian endianness = std::endian::native>
    inline void read(T *elem)
        requires byte_size<element_t>
    {
        read_n(reinterpret_cast<element_t *>(elem->raw()), T::size);
        elem->template endian<endianness>();
    }

    inline void write_single(const element_t elem)
    {
        buffer[write_index()] = elem;
        state.write_cursor++;

        state.write_count++;
    }

    inline void write_n(const element_t *elem_array, std::size_t count)
    {
        std::size_t max_contiguous;
        std::size_t to_write;

        assert(count > 0);

        while (count)
        {
            /*
             * We can only write from the current index to the end of the
             * underlying, linear buffer.
             */
            max_contiguous = depth - write_index();
            to_write = std::min(max_contiguous, count);

            /* Copy the bytes (elements -> buffer). */
            std::memcpy(&(buffer.data()[write_index()]), elem_array,
                        to_write * sizeof(element_t));

            count -= to_write;
            state.write_cursor += to_write;
            elem_array += to_write;

            state.write_count += to_write;
        }
    }

    inline std::size_t read_index(void)
    {
        return state.read_cursor % depth;
    }

    inline element_t peek(void)
    {
        return buffer[read_index()];
    }

    inline void read_single(element_t &elem)
    {
        elem = peek();
        state.read_cursor++;

        state.read_count++;
    }

    inline element_t read_single(void)
    {
        element_t elem;
        read_single(elem);
        return elem;
    }

    inline void read_n(element_t *elem_array, std::size_t count)
    {
        std::size_t max_contiguous;
        std::size_t to_read;

        assert(count > 0);

        while (count)
        {
            /*
             * We can only read from the current index to the end of the
             * underlying, linear buffer.
             */
            max_contiguous = depth - read_index();
            to_read = std::min(max_contiguous, count);

            /* Copy the bytes (buffer -> elements). */
            if (elem_array)
            {
                std::memcpy(elem_array, &(buffer.data()[read_index()]),
                            to_read * sizeof(element_t));
                elem_array += to_read;
            }

            count -= to_read;
            state.read_cursor += to_read;

            state.read_count += to_read;
        }
    }

    void poll_metrics(uint32_t &_read_count, uint32_t &_write_count,
                      bool reset = true)
    {
        _read_count = state.read_count;
        _write_count = state.write_count;
        if (reset)
        {
            state.read_count = 0;
            state.write_count = 0;
        }
    }

  protected:
    std::array<element_t, depth> buffer;

    BufferState state;
};

}; // namespace Coral
