/*
 * Thread::run for Nougat blobs that start threads without a name, which Q's libutils
 * aborts on. patch-shims.sh renames their Thread::run import (calls and vtable slots)
 * to this same-length symbol.
 */

#include <utils/Thread.h>

extern "C" android::status_t shim_android_Thread_run_name(android::Thread* thread,
                                                         const char* name, int32_t priority,
                                                         size_t stack) {
    // Qualified, not virtual: the renamed symbol also fills the blobs' vtable slots
    // for run(), so a virtual call would come straight back here.
    return thread->android::Thread::run(name ? name : "mtk-thread", priority, stack);
}
