#ifndef EL__ECMASCRIPT_QUICKJS_COLLECTION_H
#define EL__ECMASCRIPT_QUICKJS_COLLECTION_H

#include <quickjs/quickjs.h>

#ifdef __cplusplus
extern "C" {
#endif

JSValue getCollection(JSContext *ctx, void *node);

#ifdef __cplusplus
}
#endif

#endif
