#pragma once

/* toolchain */
#include <cassert>
#include <cstdint>
#include <limits>

/* internal */
#include "../PcBufferWriter.h"

namespace Coral::Cobs
{

static constexpr uint8_t zero_pointer_max =
    std::numeric_limits<uint8_t>::max();

uint8_t next_zero_distance(const uint8_t *data, std::size_t length,
                           bool skip_self = false);

class MessageEncoder
{
  public:
    MessageEncoder(const void *_data = nullptr, std::size_t _length = 0)
        : data(nullptr), length(0), bytes_sent(0), messages_sent(0),
          state(not_initialized)
    {
        /* Advance to the start state if we were constructed with real data. */
        if (_data and _length)
        {
            stage((const uint8_t *)_data, _length);
        }
    }

    /* Attempt to stage a message for encoding. */
    Result stage(const uint8_t *_data, std::size_t _length);
    Result stage(const char *_data, std::size_t _length);

    /*
     * Make as much encoding progress as possible. Returns true when the staged
     * message is completely encoded.
     *
     * Returns true only once per message.
     *
     * Returns false when the message isn't fully encoded, but the buffer
     * writer is out of space. This allows better de-coupling of an underlying
     * buffer-writer's storage size, and arbitrarily large messages to encode.
     *
     * An assertion fails if a message is not currently staged.
     */
    template <class T, typename element_t = std::byte>
    bool encode(PcBufferWriter<T, element_t> &writer)
    {
        /* Attempting to encode with no staged message is a usage bug. */
        assert(state != not_initialized);

        bool can_continue = true;

        while (state != complete and can_continue)
        {
            switch (state)
            {
            case start:
                /* Initialize the first zero pointer if necessary. */
                if (zero_pointer == 0)
                {
                    zero_pointer = next_zero_distance(data, length);
                }

                if ((can_continue = ToBool(writer.push(zero_pointer))))
                {
                    /* Ensure the last-pointer kind is set correctly. */
                    last_pointer_kind = (zero_pointer == zero_pointer_max)
                                            ? pointer_to_pointer
                                            : pointer_to_data;

                    /* Don't move the message buffer forward. */
                    advance_message(true);
                    state = encode_data;
                }

                break;

            case encode_zero:
                can_continue = handle_encode_zero(writer);
                break;

            case encode_data:
                can_continue = handle_encode_data(writer);
                break;

            case encode_delimeter:
                if ((can_continue = ToBool(writer.push(0))))
                {
                    state = complete;
                }
                break;

            /*
             * Not being initialized is a bug, and the complete state breaks us
             * out of the loop.
             */
            default:                     /* LCOV_EXCL_LINE */
                __builtin_unreachable(); /* LCOV_EXCL_LINE */
            }
        }

        bool result = state == complete;

        if (result)
        {
            assert(zero_pointer == 0);
            assert(length == 0);

            state = not_initialized;
            messages_sent++;
        }

        return result;
    }

    void stats(uint32_t &buffer_load, uint32_t &bytes_count,
               uint16_t &messages_count)
    {
        buffer_load = length;
        bytes_count = bytes_sent;
        messages_count = messages_sent;
    }

  protected:
    const uint8_t *data;
    std::size_t length;
    uint32_t bytes_sent;
    uint16_t messages_sent;

    /*
     * Always start by encoding the first zero.
     */
    enum State
    {
        not_initialized,
        start,
        encode_zero,
        encode_data,
        encode_delimeter,
        complete,
    };
    State state;

    /*
     * The first zero starts out with an unknown value.
     */
    enum ZeroPointerState
    {
        empty,
        pointer_to_data,
        pointer_to_end,
        pointer_to_pointer,
    };
    ZeroPointerState zero_pointer_state = empty;
    ZeroPointerState last_pointer_kind = empty;

    /*
     * When a zero needs to be written: holds the distance to the next zero.
     *
     * Decremented every time message bytes are processed, until it reaches
     * zero, indicating that it should be re-computed and this new value should
     * be written to the output stream..
     */
    uint8_t zero_pointer = 0;

    void advance_message(bool only_zero_pointer = false,
                         std::size_t count = 1);

    template <class T, typename element_t = std::byte>
    bool handle_zero_pointer(PcBufferWriter<T, element_t> &writer)
    {
        bool handled = false;

        bool is_overhead = last_pointer_kind == pointer_to_pointer;

        switch (zero_pointer_state)
        {
        case empty:
            zero_pointer =
                next_zero_distance(data, length, !is_overhead /* skip self */);
            zero_pointer_state = pointer_kind();

            /* See if we can write the newly populated zero pointer. */
            return handle_zero_pointer(writer);

        case pointer_to_data:
        case pointer_to_pointer:
        case pointer_to_end:
            /*
             * A 'pointer to the end' should never be followed by another zero
             * pointer.
             */
            assert(last_pointer_kind != pointer_to_end);

            if ((handled = ToBool(writer.push(zero_pointer))))
            {
                /* If this isn't an overhead pointer, we should be on a zero.
                 */
                assert(is_overhead or *data == 0);

                /* Only advance the pointer if this pointer is overhead. */
                advance_message(is_overhead);
            }
            break;
        }

        /* Reset the state machine for the next invocation. */
        if (handled)
        {
            last_pointer_kind = zero_pointer_state;
            zero_pointer_state = empty;
        }

        return handled;
    }

    template <class T, typename element_t = std::byte>
    bool handle_encode_zero(PcBufferWriter<T, element_t> &writer)
    {
        bool can_continue = handle_zero_pointer(writer);

        if (can_continue)
        {
            /* If there's no more data, encode the delimeter. */
            state = (length) ? encode_data : encode_delimeter;
        }

        return can_continue;
    }

    template <class T, typename element_t = std::byte>
    bool handle_encode_data(PcBufferWriter<T, element_t> &writer)
    {
        bool can_continue = true;

        /*
         * If we've advanced towards an expected zero (or extra zero
         * pointer), go handle it.
         */
        if (zero_pointer == 0)
        {
            state = encode_zero;
        }

        /* Write data until the next zero. */
        else if ((can_continue = ToBool(writer.push_n(data, zero_pointer))))
        {
            advance_message(false, zero_pointer);

            /*
             * Keep processing data if there's more left. Since we processed
             * the entire data chunk (until the next zero), we know a zero
             * comes next.
             */
            state = (length) ? encode_zero : encode_delimeter;
        }

        return can_continue;
    }

    ZeroPointerState pointer_kind(void);
};

}; // namespace Coral::Cobs
