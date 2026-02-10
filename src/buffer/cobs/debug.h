#pragma once

/* toolchain */
#include <cassert>
#include <cstdio>
#include <cstring>

/* internal */
#include "Decoder.h"

namespace Coral
{

void dump_buffer(const uint8_t *data, std::size_t size);

template <std::size_t _message_mtu, typename _element_t = std::byte>
void register_message_validator(
    Cobs::MessageDecoder<_message_mtu, _element_t> &decoder,
    bool &message_seen, const uint8_t *&expected, std::size_t &expected_size)
{
    decoder.set_message_callback(
        [&expected, &expected_size, &message_seen](
            const std::array<uint8_t, _message_mtu> &data, std::size_t size) {
            printf("\nOutput   - (%zu)\n", size);
            dump_buffer(data.data(), size);
            printf("Exepcted - (%zu)\n", expected_size);
            dump_buffer(expected, expected_size);

            /* Only expect to see one message. */
            assert(not message_seen);

            assert(size == expected_size);
            assert(std::memcmp(data.data(), expected, size) == 0);

            message_seen = true;
        });
}

} // namespace Coral
