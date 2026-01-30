/**
 * \file
 * \brief A buffer implementation optimized for multi-element transactions.
 */
#pragma once

/* internal */
#include "../result.h"
#include "CircularBuffer.h"

namespace Coral
{

template <std::size_t depth, std::size_t max_messages,
          byte_size element_t = std::byte>
class MessageBuffer : public CircularBuffer<depth, element_t>
{
  public:
    class MessageContext
    {
      public:
        MessageContext(MessageBuffer *_buf) : max(_buf->space()), buf(_buf)
        {
            /* Lock buffer, reset write count and determine maximum message
             * size. */
            buf->locked = true;
            buf->write_count();
        }

        ~MessageContext()
        {
            auto len = buf->write_count();

            /* Track message if written length is within bounds, otherwise
             * reset buffer due to overflow. */
            if (len <= max)
            {
                buf->add_message(len);
            }
            else
            {
                buf->clear();
            }

            buf->locked = false;
        }

        template <std::endian endianness, std::integral T>
        inline std::size_t custom(const std::byte *elem, std::size_t length)
        {
            std::size_t result = 0;
            result += buf->template write<T, endianness>(0);
            result += buf->template write_n(elem, length);
            return result;
        }

        template <std::endian endianness, ifgen_struct T>
        inline std::size_t point(const T *elem)
        {
            std::size_t result = 0;
            result += buf->template write<decltype(T::id), endianness>(T::id);
            result += buf->template write<T, endianness>(elem);
            return result;
        }

        const std::size_t max;

      protected:
        MessageBuffer *buf;
    };

    MessageBuffer()
        : CircularBuffer<depth, element_t>(), message_sizes(), num_messages(0),
          data_size(0), locked(false)
    {
    }

    MessageContext context(void)
    {
        return MessageContext(this);
    }

    inline std::size_t space(void)
    {
        return (data_size < depth) ? depth - data_size : 0;
    }

    inline bool full(std::size_t check = 0)
    {
        return num_messages >= max_messages or (data_size + check > depth);
    }

    Result put_message(const element_t *data, std::size_t len)
    {
        /* Need room for message size element and space in data buffer. */
        auto result = len and not locked and not full(len);

        if (result)
        {
            this->write_n(data, len);
            add_message(len);
        }

        return ToResult(result);
    }

    Result get_message(element_t *data, std::size_t &len)
    {
        bool result = not locked and not empty();

        if (result)
        {
            len = remove_message();
            this->read_n(data, len);
        }

        return ToResult(result);
    }

    inline bool empty()
    {
        return num_messages == 0;
    }

    inline void clear(void)
    {
        this->reset();
        message_sizes.reset();
        /* Could track drops at some point. */
        num_messages = 0;
        data_size = 0;
    }

  protected:
    CircularBuffer<max_messages, std::size_t> message_sizes;
    std::size_t num_messages;
    std::size_t data_size;
    bool locked;

    inline void add_message(std::size_t len)
    {
        message_sizes.write_single(len);
        num_messages++;
        data_size += len;
    }

    inline auto remove_message(void)
    {
        auto len = message_sizes.read_single();
        num_messages--;
        data_size -= len;
        return len;
    }
};

} // namespace Coral
