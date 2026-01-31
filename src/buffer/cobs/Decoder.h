#pragma once

/* toolchain */
#include <functional>
#include <limits>

/* internal */
#include "../PcBufferReader.h"

namespace Coral::Cobs
{

template <std::size_t message_mtu> class MessageDecoder
{
  public:
    /*
     * A callback prototype for handling fully decoded messages.
     */
    using MessageCallback = std::function<void(
        const std::array<uint8_t, message_mtu> &, std::size_t)>;

    MessageDecoder(MessageCallback _callback = nullptr)
        : message(), message_index(0), message_breached_mtu(false),
          zero_pointer(0), zero_pointer_overhead(true), bytes_dropped(0),
          message_count(0), callback(_callback)
    {
    }

    void set_message_callback(MessageCallback _callback)
    {
        /* Don't allow double assignment. */
        assert(_callback and callback == nullptr);
        callback = _callback;
    }

    template <class T, typename element_t = std::byte>
    void dispatch(PcBufferReader<T, element_t> &reader)
    {
        uint8_t current;
        bool can_continue;

        /*
         * There's only as much work to do as there is data ready to be read
         * from the buffer.
         */
        while ((can_continue = ToBool(reader.pop(current))))
        {
            /*
             * If we expect zero and land on one. The current message is fully
             * decoded. Service the message callback, which will also reset
             * decoder state.
             */
            if (zero_pointer == 0 and current == 0)
            {
                service_callback();
            }

            /*
             * If we land on a zero but didn't expect to, everything in the
             * current message buffer needs to be discarded.
             */
            else if (current == 0)
            {
                discard();
                reset();
            }

            /*
             * Decode a zero, and refill the zero pointer with the current
             * value.
             */
            else if (zero_pointer == 0)
            {
                /*
                 * If we're expecting an overhead pointer, don't add a data
                 * byte.
                 *
                 * The next zero pointer is also overhead, if the current
                 * pointer has the maximum value.
                 */
                if (zero_pointer_overhead)
                {
                    zero_pointer_overhead =
                        current == std::numeric_limits<uint8_t>::max();
                }

                /* Encode a data zero. */
                else
                {
                    add_to_message(0);
                }

                /* Count the current byte we just read. */
                zero_pointer = current - 1;
            }

            /* Regular data byte. */
            else
            {
                add_to_message(current);
                zero_pointer--;
            }
        }
    }

    void stats(uint32_t &buffer_load, uint32_t &_bytes_dropped,
               uint16_t &messages_count)
    {
        buffer_load = message_index;
        messages_count = message_count;
        _bytes_dropped = bytes_dropped;
    }

  protected:
    /* Message state. */
    std::array<uint8_t, message_mtu> message;
    std::size_t message_index;
    bool message_breached_mtu;

    /* Zero-pointer state. */
    uint8_t zero_pointer;
    bool zero_pointer_overhead;

    /* Metrics. */
    uint32_t bytes_dropped;
    uint16_t message_count;

    /*
     * Message callback.
     */
    MessageCallback callback;

    void service_callback(void)
    {
        if (callback and message_index and not message_breached_mtu)
        {
            message_count++;
            callback(message, message_index);
        }

        /* Reset decoder. */
        reset();
    }

    /*
     * Internal state modifiers.
     */

    void reset(void)
    {
        /* Reset message state. */
        message_index = 0;
        message_breached_mtu = false;

        /* Reset zero-pointer state (first pointer is always overhead). */
        zero_pointer = 0;
        zero_pointer_overhead = true;
    }

    void discard(void)
    {
        /* Consume all data bytes as dropped bytes. */
        bytes_dropped += message_index;
        message_index = 0;
    }

    void add_to_message(uint8_t value)
    {
        /* Discard all current data if we hit the MTU ceiling. */
        if (message_index >= message_mtu and not message_breached_mtu)
        {
            message_breached_mtu = true;
            discard();
        }

        /* If we haven't reset since breaching MTU, increment drop count. */
        else if (message_breached_mtu)
        {
            bytes_dropped++;
        }

        /* Regular, valid message byte. */
        else
        {
            message[message_index++] = value;
        }
    }
};

}; // namespace Coral::Cobs
