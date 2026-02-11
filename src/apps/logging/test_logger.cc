#ifdef NDEBUG
#undef NDEBUG
#endif

/* internal */
#include "ContextLock.h"
#include "logging/macros.h"

/* toolchain */
#include <cassert>

static void test_return_if(void)
{
    Coral::NoopLock lock;
    LogReturnIf(true);
}

static int test_return_val_if(void)
{
    LogReturnValIf(true, 5);
}

static void test_return_if_not(void)
{
    LogReturnIfNot(false);
}

static int test_return_val_if_not(void)
{
    LogReturnValIfNot(false, 5);
}

static Coral::Result test_log_fail_if(void)
{
    LogFailIf(true);
}

static Coral::Result test_log_fail_if_not(void)
{
    LogFailIfNot(false);
}

static Coral::Result test_log_success_if(void)
{
    LogSuccessIf(true);
}

static Coral::Result test_log_success_if_not(void)
{
    LogSuccessIfNot(false);
}

static void test_logging(void)
{
    LogIf(true);
    LogIf(1);

    LogErrnoIf(true);
    LogErrnoIf(1);

    test_return_if();

    assert(test_return_val_if() == 5);

    LogIfNot(false);
    LogErrnoIfNot(false);

    test_return_if_not();
    assert(test_return_val_if_not() == 5);

    assert((!1) == 0);
    assert(1 == (!0));

    assert(not test_log_fail_if());
    assert(test_log_success_if());

    assert(not test_log_fail_if_not());
    assert(test_log_success_if_not());
}

static Coral::Result test_fail_if(void)
{
    FailIf(true);
}

static Coral::Result test_fail_if_not(void)
{
    FailIfNot(false);
}

static Coral::Result test_success_if(void)
{
    SuccessIf(true);
}

static Coral::Result test_success_if_not(void)
{
    SuccessIfNot(false);
}

static Coral::Result test_result(void)
{
    assert(SUCCESS);
    assert(not FAIL);

    assert(not test_fail_if());
    assert(not test_fail_if_not());
    assert(test_success_if());
    assert(test_success_if_not());

    return SUCCESS;
}

int main(void)
{
    using namespace Coral;

    stderr_logger().log("Hello, world! %d\n", 1);
    stderr_logger().log("Hello, world! %d\n", 2);
    stderr_logger().log("Hello, world! %d\n", 3);

    LogErrno;

    test_logging();
    assert(test_result());

    CORAL_LOGGER.log("Nominal exit.\n");
    return 0;
}
