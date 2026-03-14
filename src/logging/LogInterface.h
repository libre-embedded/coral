/**
 * \file
 * \brief A human-readable output-logging interface.
 */
#pragma once

/* toolchain */
#include <cstdarg>

namespace Coral
{

template <class T> class LogInterface
{
  public:
    void log(const char *fmt, ...) __attribute__((format(printf, 2, 3)))
    {
        va_list args;
        va_start(args, fmt);
        vlog(fmt, args);
        va_end(args);
    }

    void vlog(const char *fmt, va_list args)
    {
        static_cast<T *>(this)->vlog_impl(fmt, args);
    }

    bool flush = false;
};

template <class T> class HasLogInterface
{
  public:
    using Logger = LogInterface<T>;

    HasLogInterface(Logger *_logger = nullptr) : logger(_logger)
    {
    }

    void log(Logger *_logger, const char *fmt, ...)
        __attribute__((format(printf, 3, 4)))
    {
        auto logger = normalize_log(_logger);

        if (logger)
        {
            va_list args;
            va_start(args, fmt);
            logger->vlog(fmt, args);
            va_end(args);
        }
    }

  protected:
    Logger *logger;

    Logger *normalize_log(Logger *_logger = nullptr)
    {
        if (not _logger)
        {
            _logger = logger;
        }

        return _logger;
    }
};

}; // namespace Coral
