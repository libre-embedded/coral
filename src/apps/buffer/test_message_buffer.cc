#ifdef NDEBUG
#undef NDEBUG
#endif

/* internal */
#include "buffer/MessageBuffer.h"

/* toolchain */
#include <iostream>
#include <stdfloat>

static constexpr std::size_t buffer_size = 256;

using CircBuffer = Coral::CircularBuffer<buffer_size>;

template <typename T, std::endian endianness = std::endian::native>
void loopback_test(CircBuffer circ_buf, T value)
{
    circ_buf.write<T, endianness>(value);
    std::cout << "wrote: '" << value << "'" << std::endl;
    T compare = circ_buf.read<T, endianness>();
    std::cout << "read:  '" << compare << "'" << std::endl;
    assert(compare == value);
}

enum class TestEnum : int16_t
{
    A,
    B,
    C
};

inline std::ostream &operator<<(std::ostream &stream, TestEnum instance)
{
    stream << std::to_underlying(instance);
    return stream;
}

int main(void)
{
    using namespace Coral;

    CircBuffer circ_buf;
    loopback_test<int8_t>(circ_buf, -5);
    loopback_test<int8_t, std::endian::big>(circ_buf, -6);
    loopback_test<int8_t, std::endian::little>(circ_buf, -7);

    loopback_test<char>(circ_buf, 'a');
    loopback_test<char, std::endian::big>(circ_buf, 'b');
    loopback_test<char, std::endian::little>(circ_buf, 'c');

    loopback_test<uint16_t>(circ_buf, 1000);
    loopback_test<uint16_t, std::endian::big>(circ_buf, 3000);
    loopback_test<uint16_t, std::endian::little>(circ_buf, 2000);

    loopback_test<Result>(circ_buf, Result::Fail);
    loopback_test<Result, std::endian::big>(circ_buf, Result::Success);
    loopback_test<Result, std::endian::little>(circ_buf, Result::Fail);

    loopback_test<TestEnum>(circ_buf, TestEnum::A);
    loopback_test<TestEnum, std::endian::big>(circ_buf, TestEnum::B);
    loopback_test<TestEnum, std::endian::little>(circ_buf, TestEnum::C);

    loopback_test<float>(circ_buf, 1.0f);
    loopback_test<float, std::endian::big>(circ_buf, -2.0f);
    loopback_test<float, std::endian::little>(circ_buf, 3.0f);
#if __STDCPP_FLOAT32_T__ == 1
    loopback_test<std::float32_t>(circ_buf, 1.0f32);
    loopback_test<std::float32_t, std::endian::big>(circ_buf, -2.0f32);
    loopback_test<std::float32_t, std::endian::little>(circ_buf, 3.0f32);
#endif

    loopback_test<double>(circ_buf, -1.0);
    loopback_test<double, std::endian::big>(circ_buf, -2.0);
    loopback_test<double, std::endian::little>(circ_buf, -3.0);
#if __STDCPP_FLOAT64_T__ == 1
    loopback_test<std::float64_t>(circ_buf, -1.0f64);
    loopback_test<std::float64_t, std::endian::big>(circ_buf, -2.0f64);
    loopback_test<std::float64_t, std::endian::little>(circ_buf, -3.0f64);
#endif

    MessageBuffer<buffer_size, 4, char> msg_buf;
    std::array<char, buffer_size> buf = {};
    std::size_t len = 0;

    assert(not msg_buf.get_message(buf.data(), len));

    len = buffer_size;
    assert(msg_buf.put_message(buf.data(), len));
    assert(not msg_buf.put_message(buf.data(), len));

    assert(msg_buf.get_message(buf.data(), len));
    assert(not msg_buf.get_message(buf.data(), len));

    return 0;
}
