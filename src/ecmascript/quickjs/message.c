/* The QuickJS MessageEvent object implementation. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elinks.h"

#include "ecmascript/ecmascript.h"
#include "ecmascript/quickjs.h"
#include "ecmascript/quickjs/message.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static JSClassID js_messageEvent_class_id;

static JSValue js_messageEvent_get_property_data(JSContext *cx, JSValueConst this_val);
static JSValue js_messageEvent_get_property_lastEventId(JSContext *cx, JSValueConst this_val);
static JSValue js_messageEvent_get_property_origin(JSContext *cx, JSValueConst this_val);
static JSValue js_messageEvent_get_property_source(JSContext *cx, JSValueConst this_val);

struct message_event {
	char *data;
	char *lastEventId;
	char *origin;
	char *source;
};

static
void js_messageEvent_finalizer(JSRuntime *rt, JSValue val)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(val);

	struct message_event *event = (struct message_event *)JS_GetOpaque(val, js_messageEvent_class_id);

	if (event) {
		mem_free_if(event->data);
		mem_free_if(event->lastEventId);
		mem_free_if(event->origin);
		mem_free_if(event->source);
		mem_free(event);
	}
}

static JSClassDef js_messageEvent_class = {
	"MessageEvent",
	js_messageEvent_finalizer
};

static const JSCFunctionListEntry js_messageEvent_proto_funcs[] = {
	JS_CGETSET_DEF("data",	js_messageEvent_get_property_data, NULL),
	JS_CGETSET_DEF("lastEventId",	js_messageEvent_get_property_lastEventId, NULL),
	JS_CGETSET_DEF("origin",	js_messageEvent_get_property_origin, NULL),
	JS_CGETSET_DEF("source",	js_messageEvent_get_property_source, NULL)
};

static JSValue
js_messageEvent_get_property_data(JSContext *ctx, JSValueConst this_val)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	struct message_event *event = (struct message_event *)(JS_GetOpaque(this_val, js_messageEvent_class_id));

	if (!event || !event->data) {
		return JS_NULL;
	}
	JSValue r = JS_NewString(ctx, event->data);

	RETURN_JS(r);
}

static JSValue
js_messageEvent_get_property_lastEventId(JSContext *ctx, JSValueConst this_val)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	struct message_event *event = (struct message_event *)(JS_GetOpaque(this_val, js_messageEvent_class_id));

	if (!event || !event->lastEventId) {
		return JS_NULL;
	}
	JSValue r = JS_NewString(ctx, event->lastEventId);

	RETURN_JS(r);
}

static JSValue
js_messageEvent_get_property_origin(JSContext *ctx, JSValueConst this_val)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	struct message_event *event = (struct message_event *)(JS_GetOpaque(this_val, js_messageEvent_class_id));

	if (!event || !event->origin) {
		return JS_NULL;
	}
	JSValue r = JS_NewString(ctx, event->origin);

	RETURN_JS(r);
}

static JSValue
js_messageEvent_get_property_source(JSContext *ctx, JSValueConst this_val)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	struct message_event *event = (struct message_event *)(JS_GetOpaque(this_val, js_messageEvent_class_id));

	if (!event || !event->source) {
		return JS_NULL;
	}
	JSValue r = JS_NewString(ctx, event->source);

	RETURN_JS(r);
}

static int lastEventId;

JSValue
get_messageEvent(JSContext *ctx, char *data, char *origin, char *source)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	static int initialized;
	/* create the element class */
	if (!initialized) {
		JS_NewClassID(&js_messageEvent_class_id);
		JS_NewClass(JS_GetRuntime(ctx), js_messageEvent_class_id, &js_messageEvent_class);
		initialized = 1;
	}
	struct message_event *event = (struct message_event *)mem_calloc(1, sizeof(*event));

	if (!event) {
		return JS_NULL;
	}
	event->data = null_or_stracpy(data);
	event->origin = null_or_stracpy(origin);
	event->source = null_or_stracpy(source);

	char id[32];

	snprintf(id, 31, "%d", ++lastEventId);
	event->lastEventId = stracpy(id);

	JSValue event_obj = JS_NewObjectClass(ctx, js_messageEvent_class_id);
	JS_SetPropertyFunctionList(ctx, event_obj, js_messageEvent_proto_funcs, countof(js_messageEvent_proto_funcs));
	JS_SetClassProto(ctx, js_messageEvent_class_id, event_obj);
	JS_SetOpaque(event_obj, event);

	JSValue rr = JS_DupValue(ctx, event_obj);
	RETURN_JS(rr);
}
