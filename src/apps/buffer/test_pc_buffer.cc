#ifdef NDEBUG
#undef NDEBUG
#endif

/* toolchain */
#include <cassert>
#include <limits>
#include <stdio.h>

/* internal */
#include "common.h"

using Buffer = PcBuffer<depth, element_t>;

void test_basic(Buffer &buf)
{
    assert(buf.head());

    buf.clear();

    element_t val = 0;
    for (std::size_t i = 0; i < depth; i++)
    {
        /* Should be able to fill the buffer. */
        assert(buf.push(val));
        val++;
    }

    /*
    PcBufferStats stats;
    buf.state.poll_stats(stats);
    assert(stats.high_watermark == depth);
    */

    /* Should not be able to add any more data. */
    assert(!buf.push(val));

    for (std::size_t i = 0; i < depth; i++)
    {
        assert(buf.pop(val));
        assert(static_cast<uint8_t>(val) ==
               (i % (std::numeric_limits<uint8_t>::max() + 1)));
    }

    /* Should not be able to read any more data. */
    assert(!buf.pop(val));
}

void test_n_push_pop(Buffer &buf)
{
    buf.clear();

    element_t tmp = 0;
    element_t data[depth];
    for (std::size_t i = 0; i < depth; i++)
    {
        data[i] = tmp++;
    }

    assert(buf.push_n(data, depth));

    element_t new_data[depth];
    assert(buf.pop_n(new_data, depth));

    /* Verify contents. */
    for (std::size_t i = 0; i < depth; i++)
    {
        assert(data[i] == new_data[i]);
    }

    assert(buf.pop_all(new_data) == 0);
    assert(buf.push_n(data, depth));
    assert(buf.pop_all(new_data) == depth);
}

void test_drop_data(Buffer &buf)
{
    /* Ensure the buffer is empty. */
    buf.pop_all();

    element_t val = 0;
    while (ToBool(buf.push(val)))
    {
        val++;
    }
    assert(buf.full());

    assert(buf.state.write_dropped == 0);
    buf.push(val, true);
    assert(buf.state.write_dropped == 1);

    buf.pop_all();
}

void test_stream_interfaces(Buffer &buf)
{
    /* Ensure the buffer is empty. */
    buf.pop_all();

    const char *data = "Hello, world! (out)\n";
    assert(buf.push_n(data, strlen(data)));

    std::cout << buf;

    /* Read input from a string stream. */
    std::stringstream("Hello, world! (in)\n") >> buf;
    std::cout << buf;
}

int main(void)
{
    Buffer buf(
        true, [](Buffer *buf) { (void)buf; }, [](Buffer *buf) { (void)buf; });
    assert(buf.empty());
    assert(!buf.full());

    test_basic(buf);
    test_n_push_pop(buf);

    Buffer buf2 = {};
    test_drop_data(buf2);

    test_stream_interfaces(buf2);

    char data = 'x';
    for (std::size_t i = 0; i < depth; i++)
    {
        buf2.push_blocking(data);
    }
    assert(buf2.full());

    /* Ensure that blocking write works. */
    buf2.set_data_available([](Buffer *buf) { buf->pop_all(); });
    buf2.push_blocking(data);

    buf2.set_data_available();

    for (std::size_t i = 0; i < depth; i++)
    {
        buf2.push_blocking(data);
    }
    assert(buf2.full());

    /* Ensure that blocking write works. */
    buf2.set_data_available([](Buffer *buf) { buf->pop_all(); });
    std::array<element_t, depth * 10> data_array = {};
    buf2.push_n_blocking(data_array);

    return 0;
}
