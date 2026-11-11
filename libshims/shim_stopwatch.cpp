/*
 * R dropped the flags argument (and field) of android::StopWatch. The Nougat
 * hwcomposer constructs StopWatch(name, clock, flags) in a buffer sized for the
 * old class (which is larger), then only calls methods, so constructing the R
 * object in place is safe.
 */
#include <new>
#include <stdint.h>

#include <utils/StopWatch.h>

static_assert(sizeof(android::StopWatch) <= 160, "R StopWatch must fit the Nougat object");

extern "C" void _ZN7android9StopWatchC1EPKcij(android::StopWatch* self, const char* name, int clock,
                                             uint32_t /* flags */) {
    new (self) android::StopWatch(name, clock);
}
