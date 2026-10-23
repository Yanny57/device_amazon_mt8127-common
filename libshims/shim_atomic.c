/*
 * Oreo's libcutils made the android_atomic_* functions static inline, so they
 * are no longer exported. Nougat-era blobs (libMali, hwcomposer, camera...)
 * still import them. Same semantics as the old libcutils implementation:
 * read-modify-write ops return the previous value with release ordering.
 */

#include <stdint.h>

int32_t android_atomic_inc(volatile int32_t* addr)
{
    return __atomic_fetch_add(addr, 1, __ATOMIC_RELEASE);
}

int32_t android_atomic_dec(volatile int32_t* addr)
{
    return __atomic_fetch_sub(addr, 1, __ATOMIC_RELEASE);
}

int32_t android_atomic_add(int32_t value, volatile int32_t* addr)
{
    return __atomic_fetch_add(addr, value, __ATOMIC_RELEASE);
}

int32_t android_atomic_and(int32_t value, volatile int32_t* addr)
{
    return __atomic_fetch_and(addr, value, __ATOMIC_RELEASE);
}

int32_t android_atomic_or(int32_t value, volatile int32_t* addr)
{
    return __atomic_fetch_or(addr, value, __ATOMIC_RELEASE);
}

int32_t android_atomic_acquire_load(volatile const int32_t* addr)
{
    return __atomic_load_n(addr, __ATOMIC_ACQUIRE);
}

int32_t android_atomic_release_load(volatile const int32_t* addr)
{
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    return __atomic_load_n(addr, __ATOMIC_RELAXED);
}

void android_atomic_acquire_store(int32_t value, volatile int32_t* addr)
{
    __atomic_store_n(addr, value, __ATOMIC_RELAXED);
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
}

void android_atomic_release_store(int32_t value, volatile int32_t* addr)
{
    __atomic_store_n(addr, value, __ATOMIC_RELEASE);
}

/* Return 0 on success (like the old libcutils), nonzero if *addr != oldvalue. */
int android_atomic_acquire_cas(int32_t oldvalue, int32_t newvalue, volatile int32_t* addr)
{
    return !__atomic_compare_exchange_n(addr, &oldvalue, newvalue, 0,
            __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
}

int android_atomic_release_cas(int32_t oldvalue, int32_t newvalue, volatile int32_t* addr)
{
    return !__atomic_compare_exchange_n(addr, &oldvalue, newvalue, 0,
            __ATOMIC_RELEASE, __ATOMIC_RELAXED);
}

int android_atomic_cas(int32_t oldvalue, int32_t newvalue, volatile int32_t* addr)
{
    return android_atomic_release_cas(oldvalue, newvalue, addr);
}
