#ifndef EL__DOCUMENT_ECMASCRIPT_LIBDOM_DOM_H
#define EL__DOCUMENT_ECMASCRIPT_LIBDOM_DOM_H

#ifdef __cplusplus
extern "C" {
#endif

#define namespace namespace_elinks

#ifdef CONFIG_LIBDOM
#include <dom/dom.h>
#include <dom/bindings/hubbub/parser.h>
#endif

#undef namespace

#ifdef __cplusplus
}
#endif

#endif
