/* internal */
#include "Encoder.h"

namespace Coral::Cobs
{

uint8_t next_zero_distance(const uint8_t *data, std::size_t length,
                           bool skip_self)
{
    uint8_t result = 0;

    /*
     * An option for ensuring that a zero at the current buffer position
     * doesn't result in this also producing 1.
     */
    if (skip_self)
    {
        assert(length);

        data++;
        length--;
    }

    while (result < zero_pointer_max - 1 and result < length)
    {
        if (data[result] == 0)
        {
            break;
        }
        result++;
    }

    /* Return 'distance', not index. */
    return result + 1;
}

Result MessageEncoder::stage(const uint8_t *_data, std::size_t _length)
{
    /*
     * Can only stage a message if the previous encoding is complete, or
     * the encoder hasn't been initialized yet.
     */
    bool result = state == not_initialized or state == complete;

    if (result)
    {
        data = _data;
        length = _length;
        state = start;
        stats_new = true;
    }

    return ToResult(result);
}

void MessageEncoder::advance_message(bool only_zero_pointer, std::size_t count)
{
    if (not only_zero_pointer)
    {
        data += count;

        assert(length >= count);
        length -= count;
    }

    assert(zero_pointer >= count);
    zero_pointer -= count;
    bytes_sent += count;
    stats_new = true;
}

MessageEncoder::ZeroPointerState MessageEncoder::pointer_kind(void)
{
    /*
     * For a zero pointer to point to end-of-frame, it must be longer than the
     * length.
     */
    if (zero_pointer > length)
    {
        return pointer_to_end;
    }

    return (zero_pointer == zero_pointer_max) ? pointer_to_pointer
                                              : pointer_to_data;
}

} // namespace Coral::Cobs
