/**
 * \file
 * \brief Interfaces for handling endianness.
 */
#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace Coral
{

/* Could be defined by ifgen common. */
template <typename T>
concept byte_size = sizeof(T) == 1;

template <byte_size T, std::endian endianness> inline void handle_endian(T &)
{
}

template <std::integral T, std::endian endianness>
inline void handle_endian(T &)
    requires(endianness == std::endian::native)
{
}

template <std::integral T, std::endian endianness>
inline void handle_endian(T &elem)
    requires(not byte_size<T>) && (endianness != std::endian::native)
{
    elem = std::byteswap(elem);
}

template <std::floating_point T, std::endian endianness>
inline void handle_endian(T &elem)
    requires(sizeof(T) == sizeof(uint32_t))
{
    handle_endian<uint32_t, endianness>(*reinterpret_cast<uint32_t *>(&elem));
}

template <std::floating_point T, std::endian endianness>
inline void handle_endian(T &elem)
    requires(sizeof(T) == sizeof(uint64_t))
{
    handle_endian<uint64_t, endianness>(*reinterpret_cast<uint64_t *>(&elem));
}

} // namespace Coral
