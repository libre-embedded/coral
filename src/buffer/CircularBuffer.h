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

template <std::size_t depth, typename element_t = std::byte,
          std::size_t alignment = sizeof(element_t)>
class CircularBuffer
{
    static_assert(depth > 0);
    static_assert(std::has_single_bit(alignment));

  public:
    static constexpr std::size_t Depth = depth;

    CircularBuffer() : buffer(), state()
    {
    }

    template <std::endian endianness, byte_size T>
    inline std::size_t write(T elem)
        requires byte_size<element_t>
    {
        return write_single(static_cast<element_t>(elem));
    }

    template <std::endian endianness, typename T>
    inline std::size_t write(T elem)
        requires byte_size<element_t> && (not byte_size<T>)
    {
        /* Parameter passed by value can be directly swapped. */
        elem = handle_endian<endianness>(elem);
        return write_n(reinterpret_cast<const element_t *>(&elem), sizeof(T));
    }

    template <std::endian endianness, ifgen_struct T>
    inline std::size_t write(const T *elem)
        requires byte_size<element_t>
    {
        /* Use a stack instance/copy for byte swapping. */
        T temp = T();
        temp.template decode<endianness>(elem->raw_ro());
        return write_n(reinterpret_cast<const element_t *>(temp.raw_ro()),
                       T::size);
    }

    template <std::endian endianness, byte_size T>
    inline T read(void)
        requires byte_size<element_t>
    {
        return static_cast<T>(read_single());
    }

    template <std::endian endianness, typename T>
    inline T read(void)
        requires byte_size<element_t> && (not byte_size<T>)
    {
        T result = T();
        read_n(reinterpret_cast<element_t *>(&result), sizeof(T));
        return handle_endian<endianness>(result);
    }

    template <std::endian endianness, ifgen_struct T>
    inline void read(T *elem)
        requires byte_size<element_t>
    {
        read_n(reinterpret_cast<element_t *>(elem->raw()), T::size);
        elem->template endian<endianness>();
    }

    inline std::size_t write_single(const element_t elem)
    {
        buffer[write_index()] = elem;
        state.write_cursor++;

        state.write_count++;
        return 1;
    }

    inline std::size_t write_n(const element_t *elem_array, std::size_t count)
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
            if (elem_array)
            {
                std::memcpy(&(buffer.data()[write_index()]), elem_array,
                            to_write * sizeof(element_t));
                elem_array += to_write;
            }

            count -= to_write;
            state.write_cursor += to_write;

            state.write_count += to_write;
        }

        return count;
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

    inline void poll_metrics(uint32_t &_read_count, uint32_t &_write_count,
                             bool reset = true)
    {
        _read_count = read_count(reset);
        _write_count = write_count(reset);
    }

    inline uint32_t write_count(bool reset = true)
    {
        auto result = state.write_count;
        if (reset)
        {
            state.write_count = 0;
        }
        return result;
    }

    inline uint32_t read_count(bool reset = true)
    {
        auto result = state.read_count;
        if (reset)
        {
            state.read_count = 0;
        }
        return result;
    }

    inline void reset(void)
    {
        state = {};
    }

    inline const element_t *head(void)
    {
        return buffer.data();
    }

  protected:
    alignas(alignment) std::array<element_t, depth> buffer;

    BufferState state;

    inline std::size_t write_index(void)
    {
        return state.write_cursor % depth;
    }

    inline std::size_t read_index(void)
    {
        return state.read_cursor % depth;
    }
};

}; // namespace Coral
