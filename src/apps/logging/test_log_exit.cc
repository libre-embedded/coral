#ifdef NDEBUG
#undef NDEBUG
#endif

/* internal */
#include "logging/macros.h"

int main(void)
{
    using namespace Coral;

    LogExitIfNot(false, 0);

    return 1;
}
