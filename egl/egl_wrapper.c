/*
 * EGL driver front for Mali r4p0 (libEGL_mali_vendor.so).
 *
 * Android 10 SystemUI's ImageWallpaper always asks for EGL_CONTEXT_PRIORITY_LEVEL_IMG,
 * without checking for EGL_IMG_context_priority. This driver doesn't implement it and
 * fails the whole context with EGL_BAD_ATTRIBUTE, leaving the wallpaper black. Drop
 * the attribute (a scheduling hint) and pass everything else through.
 *
 * libEGL looks up each EGL entry point with dlsym() and falls back to the driver's
 * eglGetProcAddress(), so only these two need to be exported here.
 */

#define LOG_TAG "egl_wrapper"

#include <dlfcn.h>
#include <pthread.h>
#include <string.h>

#include <EGL/egl.h>
#include <log/log.h>

#define VENDOR_LIB "libEGL_mali_vendor.so"
#define EGL_CONTEXT_PRIORITY_LEVEL_IMG 0x3100
#define MAX_ATTRIBS 64

typedef EGLContext (*create_context_t)(EGLDisplay, EGLConfig, EGLContext, const EGLint*);
typedef __eglMustCastToProperFunctionPointerType (*get_proc_address_t)(const char*);

static void* vendor;
static create_context_t vendor_create_context;
static get_proc_address_t vendor_get_proc_address;
static pthread_once_t vendor_once = PTHREAD_ONCE_INIT;

static void load_vendor(void) {
    vendor = dlopen(VENDOR_LIB, RTLD_NOW);
    if (!vendor) {
        ALOGE("dlopen %s: %s", VENDOR_LIB, dlerror());
        return;
    }
    vendor_create_context = (create_context_t)dlsym(vendor, "eglCreateContext");
    vendor_get_proc_address = (get_proc_address_t)dlsym(vendor, "eglGetProcAddress");
}

EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context,
                            const EGLint* attrib_list) {
    EGLint attribs[MAX_ATTRIBS + 1];
    int n = 0;

    pthread_once(&vendor_once, load_vendor);
    if (!vendor_create_context) return EGL_NO_CONTEXT;

    if (attrib_list) {
        for (; attrib_list[0] != EGL_NONE; attrib_list += 2) {
            if (attrib_list[0] == EGL_CONTEXT_PRIORITY_LEVEL_IMG) continue;
            if (n + 2 > MAX_ATTRIBS) return vendor_create_context(dpy, config, share_context,
                                                                  attrib_list);
            attribs[n++] = attrib_list[0];
            attribs[n++] = attrib_list[1];
        }
        attribs[n] = EGL_NONE;
        attrib_list = attribs;
    }
    return vendor_create_context(dpy, config, share_context, attrib_list);
}

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* procname) {
    __eglMustCastToProperFunctionPointerType f = NULL;

    pthread_once(&vendor_once, load_vendor);
    if (!vendor || !procname) return NULL;
    if (!strcmp(procname, "eglCreateContext"))
        return (__eglMustCastToProperFunctionPointerType)eglCreateContext;
    if (vendor_get_proc_address) f = vendor_get_proc_address(procname);
    if (!f) f = (__eglMustCastToProperFunctionPointerType)dlsym(vendor, procname);
    return f;
}
