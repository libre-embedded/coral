/**
 * \file
 * \brief Logging-related preprocessor macros.
 */
#pragma once

/* internal */
#include "../result.h"
#include "PrintfLogger.h"

/* toolchain */
#include <cerrno>
#include <cstdlib>
#include <cstring>

#ifndef CORAL_LOGGER
#define CORAL_LOGGER ::Coral::stderr_logger()
#endif

#ifndef LogIf
#define LogIf(x)                                                              \
    if (x)                                                                    \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogIf(" #x ")\n", __FILE__, __LINE__);        \
    }
#endif

#ifndef LogErrno
#define LogErrno                                                              \
    CORAL_LOGGER.log("%s:%d LogErrno '%s'\n", __FILE__, __LINE__,             \
                     strerror(errno));
#endif

#ifndef LogErrnoIf
#define LogErrnoIf(x)                                                         \
    if (x)                                                                    \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogErrnoIf(" #x ") '%s'\n", __FILE__,         \
                         __LINE__, strerror(errno));                          \
    }
#endif

#ifndef LogErrnoReturnIf
#define LogErrnoReturnIf(x)                                                   \
    if (x)                                                                    \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogErrnoReturnIf(" #x ") '%s'\n", __FILE__,   \
                         __LINE__, strerror(errno));                          \
        return;                                                               \
    }
#endif

#ifndef LogReturnIf
#define LogReturnIf(x)                                                        \
    if (x)                                                                    \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogReturnIf(" #x ")\n", __FILE__, __LINE__);  \
        return;                                                               \
    }
#endif

#ifndef LogFailIf
#define LogFailIf(x)                                                          \
    if (x)                                                                    \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogFailIf(" #x ")\n", __FILE__, __LINE__);    \
        return FAIL;                                                          \
    }
#endif

#ifndef LogSuccessIf
#define LogSuccessIf(x)                                                       \
    if (x)                                                                    \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogSuccessIf(" #x ")\n", __FILE__, __LINE__); \
        return SUCCESS;                                                       \
    }
#endif

#ifndef LogReturnValIf
#define LogReturnValIf(x, val)                                                \
    if (x)                                                                    \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogReturnValIf(" #x ", " #val ")\n",          \
                         __FILE__, __LINE__);                                 \
        return val;                                                           \
    }
#endif

#ifndef LogIfNot
#define LogIfNot(x)                                                           \
    if (not(x))                                                               \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogIfNot(" #x ")\n", __FILE__, __LINE__);     \
    }
#endif

#ifndef LogErrnoIfNot
#define LogErrnoIfNot(x)                                                      \
    if (not(x))                                                               \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogErrnoIfNot(" #x ") '%s'\n", __FILE__,      \
                         __LINE__, strerror(errno));                          \
    }
#endif

#ifndef LogErrnoReturnIfNot
#define LogErrnoReturnIfNot(x)                                                \
    if (not(x))                                                               \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogErrnoReturnIfNot(" #x ") '%s'\n",          \
                         __FILE__, __LINE__, strerror(errno));                \
        return;                                                               \
    }
#endif

#ifndef LogReturnIfNot
#define LogReturnIfNot(x)                                                     \
    if (not(x))                                                               \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogReturnIfNot(" #x ")\n", __FILE__,          \
                         __LINE__);                                           \
        return;                                                               \
    }
#endif

#ifndef LogFailIfNot
#define LogFailIfNot(x)                                                       \
    if (not(x))                                                               \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogFailIfNot(" #x ")\n", __FILE__, __LINE__); \
        return FAIL;                                                          \
    }
#endif

#ifndef LogSuccessIfNot
#define LogSuccessIfNot(x)                                                    \
    if (not(x))                                                               \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogSuccessIfNot(" #x ")\n", __FILE__,         \
                         __LINE__);                                           \
        return SUCCESS;                                                       \
    }
#endif

#ifndef LogReturnValIfNot
#define LogReturnValIfNot(x, val)                                             \
    if (not(x))                                                               \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogReturnValIfNot(" #x ", " #val ")\n",       \
                         __FILE__, __LINE__);                                 \
        return val;                                                           \
    }
#endif

#ifndef LogExitIf
#define LogExitIf(x, val)                                                     \
    if (x)                                                                    \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogExitIf(" #x ", " #val ")\n", __FILE__,     \
                         __LINE__);                                           \
        std::exit(val);                                                       \
    }
#endif

#ifndef LogExitIfNot
#define LogExitIfNot(x, val)                                                  \
    if (not(x))                                                               \
    {                                                                         \
        CORAL_LOGGER.log("%s:%d LogExitIfNot(" #x ", " #val ")\n", __FILE__,  \
                         __LINE__);                                           \
        std::exit(val);                                                       \
    }
#endif
