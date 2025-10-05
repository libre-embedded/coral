/**
 * \file
 * \brief A simple logger implementation that uses printf.
 */
#pragma once

/* toolchain */
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

/* internal */
#include "LogInterface.h"

namespace Coral
{

class PrintfLogger : public LogInterface<PrintfLogger>
{
  public:
    void vlog_impl(const char *fmt, va_list args)
    {
        vprintf(fmt, args);
    }
};

class FdPrintfLogger : public LogInterface<FdPrintfLogger>
{
  public:
    FdPrintfLogger(int _fd) : fd(_fd)
    {
    }

    void vlog_impl(const char *fmt, va_list args)
    {
        // not present in picolibc tiny stdio
        // vdprintf(fd, fmt, args);

        char buf[BUFSIZ];
        int n = sizeof buf;
        n = vsnprintf(buf, n, fmt, args);
        if (n > 0)
        {
            assert(write(fd, buf, n) > 0);
        }
    }

  protected:
    int fd;
};

FdPrintfLogger &stderr_logger();

}; // namespace Coral
