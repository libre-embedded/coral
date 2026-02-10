#ifdef NDEBUG
#undef NDEBUG
#endif

/* toolchain */
#include <iostream>
#include <string>

/* internal */
#include "buffer/PcBuffer.h"
#include "buffer/cobs/Encoder.h"
#include "buffer/cobs/debug.h"

using namespace Coral;

static constexpr std::size_t message_mtu = 1024;

using Decoder = Cobs::MessageDecoder<message_mtu, uint8_t>;
using Encoder = Cobs::MessageEncoder;
using PcBuf = PcBuffer<message_mtu * 2, uint8_t>;

void test_string(const std::string &message)
{
    const uint8_t *message_ptr = (const uint8_t *)message.c_str();
    std::size_t message_size = message.size();

    /* Initialize the encoder. */
    Encoder encoder = Encoder(message_ptr, message_size);

    PcBuf buffer = PcBuf();

    /* Encode the message into the buffer. */
    assert(encoder.encode(buffer));

    /* Register message validation. */
    Decoder decoder = Decoder();
    bool message_seen = false;
    register_message_validator(decoder, message_seen, message_ptr,
                               message_size);

    /* Decode the message. */
    decoder.dispatch(buffer);
    assert(message_seen);

    std::cout << "Decoded: '" << message << "'." << std::endl;
}

int main(void)
{
    test_string("Hello, world!");

    test_string("Hello, world!\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n"
                "This is a long message.\n");

    return 0;
}
