#ifdef NDEBUG
#undef NDEBUG
#endif

/* toolchain */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

/* internal */
#include "buffer/PcBuffer.h"
#include "buffer/cobs/Encoder.h"
#include "buffer/cobs/debug.h"

using namespace Coral;

static constexpr std::size_t buffer_size = 2048;

void test_zero_distances(void)
{
    uint8_t data[1024] = {};

    assert(Cobs::next_zero_distance(data, 1) == 1);
    assert(Cobs::next_zero_distance(data, 2) == 1);
    assert(Cobs::next_zero_distance(data, 3) == 1);

    data[0] = 1;

    assert(Cobs::next_zero_distance(data, 1) == 2);
    assert(Cobs::next_zero_distance(data, 2) == 2);
    assert(Cobs::next_zero_distance(data, 3) == 2);

    /* Keep the 254th byte a zero. */
    for (std::size_t i = 0; i < Cobs::zero_pointer_max - 2; i++)
    {
        data[i] = 1;
    }

    assert(Cobs::next_zero_distance(data, 1) == 2);
    assert(Cobs::next_zero_distance(data, 2) == 3);
    assert(Cobs::next_zero_distance(data, 3) == 4);
    assert(Cobs::next_zero_distance(data, 1024) == Cobs::zero_pointer_max - 1);

    /* Make the 254th byte non-zero. */
    data[Cobs::zero_pointer_max - 2] = 1;
    assert(Cobs::next_zero_distance(data, 1024) == Cobs::zero_pointer_max);
    data[Cobs::zero_pointer_max - 1] = 1;
    assert(Cobs::next_zero_distance(data, 1024) == Cobs::zero_pointer_max);
}

void verify_encode_result(const uint8_t *output, std::size_t output_size,
                          const uint8_t *expected, std::size_t expected_size,
                          const char *name)
{
    if (name)
    {
        assert(expected_size and expected);

        printf("Output   - (%zu)\n", output_size);
        dump_buffer(output, output_size);

        printf("\nExepcted - (%zu)\n", expected_size);
        dump_buffer(expected, expected_size);
    }

    if (expected_size and expected)
    {
        assert(output_size == expected_size);

        /* Confirm the contents. */
        assert(std::memcmp(output, expected, expected_size) == 0);
    }

    if (name)
    {
        assert(expected_size and expected);
        printf("\n(%s passed)\n", name);
    }
}

void encoder_scenario(const uint8_t *input, std::size_t input_size,
                      const uint8_t *expected = nullptr,
                      std::size_t expected_size = 0,
                      const char *name = nullptr)
{
    Cobs::MessageEncoder encoder = Cobs::MessageEncoder(input, input_size);

    /* Shouldn't be able to stage a new message. */
    assert(not encoder.stage(input, input_size));

    /* Encode the message. */
    PcBuffer<buffer_size * 2, uint8_t> buffer =
        PcBuffer<buffer_size * 2, uint8_t>();
    assert(encoder.encode(buffer));

    /* Extract the result. */
    uint8_t output[buffer_size * 2];
    std::size_t output_size = buffer.pop_all(output);

    verify_encode_result(output, output_size, expected, expected_size, name);

    /* Encode the message again, but with a minimally-sized buffer. */
    PcBuffer<254, uint8_t> small_buffer = PcBuffer<254, uint8_t>();

    /* Should be able to stage a new message. */
    assert(encoder.stage(input, input_size));

    uint8_t *output_ptr = output;
    output_size = 0;
    std::size_t curr_chunk;

    while (!encoder.encode(small_buffer))
    {
        curr_chunk = small_buffer.pop_all(output_ptr);

        /* There should always be data to pop. */
        assert(curr_chunk); /* LCOV_EXCL_LINE */

        output_ptr += curr_chunk;
        output_size += curr_chunk;
    }

    /* Process the last chunk. */
    curr_chunk = small_buffer.pop_all(output_ptr);
    output_ptr += curr_chunk;
    output_size += curr_chunk;

    /* Message should once again be correct. */
    verify_encode_result(output, output_size, expected, expected_size, name);

    uint32_t buffer_load;
    uint32_t sent_count;
    uint16_t messages_count;
    assert(encoder.stats(&buffer_load, &sent_count, &messages_count));
    assert(not buffer_load);
    assert(sent_count);
    assert(messages_count);
}

int main(void)
{
    test_zero_distances();

    uint8_t input[buffer_size] = {0};
    /* Make sure there's room for overhead. */
    uint8_t output[buffer_size * 2] = {0};
    std::size_t input_size;
    std::size_t output_size;

    /*
     * See:
     * https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing#Encoding_examples
     */

    /*
     * Example 1.
     */
    input_size = 1;
    output_size = 3;
    output[0] = 1;
    output[1] = 1;
    encoder_scenario(input, input_size, output, output_size, "example 1");

    /*
     * Example 2.
     */
    input_size = 2;
    output_size = 4;
    output[2] = 1;
    encoder_scenario(input, input_size, output, output_size, "example 2");

    /*
     * Example 3.
     */
    input_size = 3;
    output_size = 5;

    input[1] = 0x11;

    output[0] = 1;
    output[1] = 2;
    output[2] = 0x11;
    output[3] = 1;
    output[4] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 3");

    /*
     * Example 4.
     */
    input_size = 4;
    output_size = 6;

    input[0] = 0x11;
    input[1] = 0x22;
    input[2] = 0;
    input[3] = 0x33;

    output[0] = 3;
    output[1] = 0x11;
    output[2] = 0x22;
    output[3] = 2;
    output[4] = 0x33;
    output[5] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 4");

    /*
     * Example 5.
     */

    input[0] = 0x11;
    input[1] = 0x22;
    input[2] = 0x33;
    input[3] = 0x44;

    output[0] = 5;
    output[1] = 0x11;
    output[2] = 0x22;
    output[3] = 0x33;
    output[4] = 0x44;
    output[5] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 5");

    /*
     * Example 6.
     */

    input[0] = 0x11;
    input[1] = 0;
    input[2] = 0;
    input[3] = 0;

    output[0] = 2;
    output[1] = 0x11;
    output[2] = 1;
    output[3] = 1;
    output[4] = 1;
    output[5] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 6");

    /*
     * Example 7.
     */
    input_size = 254;
    output_size = 256;
    for (std::size_t i = 0; i < input_size; i++)
    {
        input[i] = i + 1;
    }

    output[0] = 255;
    for (std::size_t i = 0; i < input_size; i++)
    {
        output[i + 1] = i + 1;
    }
    output[255] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 7");

    /*
     * Example 8.
     */
    input_size = 255;
    output_size = 257;
    for (std::size_t i = 0; i < input_size; i++)
    {
        input[i] = i;
    }

    output[0] = 1;
    output[1] = 0xff;
    for (std::size_t i = 0; i < input_size; i++)
    {
        output[i + 2] = i + 1;
    }
    output[256] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 8");

    /*
     * Example 9.
     */
    output_size = 258;
    for (std::size_t i = 0; i < input_size; i++)
    {
        input[i] = i + 1;
    }

    output[0] = 0xff;
    for (std::size_t i = 0; i < input_size; i++)
    {
        output[i + 1] = i + 1;
    }
    output[255] = 2;
    output[256] = 0xff;
    output[257] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 9");

    /*
     * Example 10.
     */
    for (std::size_t i = 0; i < input_size; i++)
    {
        input[i] = i + 2;
    }

    output[0] = 0xff;
    for (std::size_t i = 0; i < input_size; i++)
    {
        output[i + 1] = i + 2;
    }
    output[255] = 1;
    output[256] = 1;
    output[257] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 10");

    /*
     * Example 11.
     */
    output_size = 257;
    for (std::size_t i = 0; i < input_size; i++)
    {
        input[i] = i + 3;
    }

    output[0] = 0xfe;
    for (std::size_t i = 0; i < input_size; i++)
    {
        output[i + 1] = i + 3;
    }
    output[254] = 2;
    output[255] = 1;
    output[256] = 0;

    encoder_scenario(input, input_size, output, output_size, "example 11");

    /*
     * Example 8 (longer).
     */
    input_size = 256;
    output_size = 259;
    for (std::size_t i = 0; i < input_size; i++)
    {
        input[i] = i;
    }

    output[0] = 1;
    output[1] = 0xff;
    for (std::size_t i = 0; i < input_size; i++)
    {
        output[i + 2] = i + 1;
    }
    output[256] = 2;
    output[257] = 0xff;
    output[258] = 0;

    encoder_scenario(input, input_size, output, output_size,
                     "example 8 (longer)");

    /*
     * CHAOS TEST. Run the encoder at every possible buffer size with the
     * following datasets:
     * - all zeros
     * - all ones
     * - random data
     *
     * Make sure the program doesn't crash and that we don't violate
     * any assertions.
     */
    int num_iterations = buffer_size - 1;
    for (int i = 0; i < num_iterations; i++)
    {
        /* Run the scenario at every buffer size. */
        input_size = i + 1;

        /* Data with no zeroes. */
        std::memset(input, 1, input_size);
        encoder_scenario(input, input_size);

        /* Data with all zeroes. */
        std::memset(input, 0, input_size);
        encoder_scenario(input, input_size);

        /* Data with random contents zeroes. */
        for (std::size_t j = 0; j < input_size; j++)
        {
            input[j] = rand() % std::numeric_limits<uint8_t>::max();
        }
        encoder_scenario(input, input_size);
    }
    printf("All %d random encoder tasks completed.\n", num_iterations);

    /* For coverage. */
    Cobs::MessageEncoder encoder = Cobs::MessageEncoder(input, input_size);
    encoder.stage("test", strlen("test"));

    return 0;
}
