/**
 * \file
 * \brief A command processor implementation for strings.
 */
#pragma once

/* internal */
#include "../buffer/PcBuffer.h"

namespace Coral
{

static constexpr std::size_t default_max_args = 32;

template <std::size_t depth, typename element_t = char,
          std::size_t max_args = default_max_args>
class StringCommandProcessor
{
  public:
    using Buffer = PcBuffer<depth, element_t>;
    using String = std::basic_string<element_t>;

    using Handler = std::function<void(const element_t **, std::size_t)>;

    static constexpr element_t null = '\0';
    static constexpr element_t default_line_delim = '\n';
    static constexpr element_t default_word_delim = ' ';

    StringCommandProcessor(Buffer &_input, Handler _handler = nullptr,
                           bool auto_poll = false,
                           element_t _line_delim = default_line_delim,
                           element_t _word_delim = default_word_delim)
        : handler(_handler), input(_input), line_delim(_line_delim),
          word_delim(_word_delim)
    {
        reset();

        if (auto_poll)
        {
            set_auto_poll();
        }
    }

    void set_auto_poll()
    {
        /* Poll the buffer whenever there's data. */
        input.set_data_available([this](Buffer *buf) {
            (void)buf;
            poll();
        });
    }

    void set_handler(Handler _handler = nullptr)
    {
        handler = _handler;
    }

    void reset()
    {
        std::memset(line, 0, depth);
        index = 0;
    }

    std::size_t poll()
    {
        std::size_t num_lines = 0;

        while (not input.empty())
        {
            input.pop_n(&line[index], 1);

            if (line[index] != line_delim)
            {
                index++;

                /*
                 * Drop the current buffer if it becomes full before we see
                 * the delimeter.
                 */
                if (index >= depth - 1)
                {
                    reset();
                }
            }

            /* Only take action if we have some input. */
            else if (index)
            {
                line[index] = null;
                process(line, index);
                reset();
                num_lines++;
            }
        }

        return num_lines;
    }

    inline void process(String &&line)
    {
        process(line.data(), line.size());
    }

    inline void process(String &line)
    {
        process(line.data(), line.size());
    }

    void process(element_t *line, std::size_t length)
    {
        num_args = 0;
        std::size_t token_start = 0;
        std::size_t token_len = 0;

        for (std::size_t i = 0; i < length; i++)
        {
            if (line[i] == word_delim)
            {
                line[i] = null;

                if (token_len)
                {
                    args[num_args++] = &line[token_start];
                    token_len = 0;
                }

                token_start = i + 1;
            }
            else
            {
                token_len++;
            }
        }

        if (token_len)
        {
            args[num_args] = &line[token_start];
            num_args++;
        }

        /* Handle the command. */
        if (handler and num_args)
        {
            handler(args, num_args);
        }
    }

    Handler handler;

  protected:
    Buffer &input;

    /* Line parsing. */
    element_t line_delim;
    element_t line[depth];
    std::size_t index;

    /* Word parsing. */
    element_t word_delim;
    const element_t *args[max_args];
    std::size_t num_args;
};

} // namespace Coral
