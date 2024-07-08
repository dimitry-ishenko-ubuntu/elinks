#ifndef EL__ECMASCRIPT_QUICKJS_LOCALSTORAGE_H
#define EL__ECMASCRIPT_QUICKJS_LOCALSTORAGE_H

#include <quickjs/quickjs.h>

#ifdef __cplusplus
extern "C" {
#endif

int js_localstorage_init(JSContext *ctx);

#ifdef __cplusplus
}
#endif

#endif
