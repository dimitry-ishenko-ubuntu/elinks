#ifndef EL__ECMASCRIPT_SPIDERMONKEY_KEYBOARD_H
#define EL__ECMASCRIPT_SPIDERMONKEY_KEYBOARD_H

#include "ecmascript/spidermonkey/util.h"

struct term_event;

extern JSClass keyboardEvent_class;
extern JSPropertySpec keyboardEvent_props[];
bool keyboardEvent_constructor(JSContext* ctx, unsigned argc, JS::Value* vp);

JSObject *get_keyboardEvent(JSContext *ctx, struct term_event *ev);

#endif
