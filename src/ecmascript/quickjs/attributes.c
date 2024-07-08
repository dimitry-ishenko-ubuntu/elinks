/* The QuickJS attributes object implementation. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifdef CONFIG_LIBDOM
#include <dom/dom.h>
#include <dom/bindings/hubbub/parser.h>
#endif

#include "elinks.h"

#include "document/libdom/corestrings.h"
#include "ecmascript/ecmascript.h"
#include "ecmascript/quickjs/mapa.h"
#include "ecmascript/quickjs.h"
#include "ecmascript/quickjs/attr.h"
#include "ecmascript/quickjs/attributes.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

void *map_attributes;
void *map_rev_attributes;

static void *
js_attributes_GetOpaque(JSValueConst this_val)
{
	REF_JS(this_val);

	return attr_find_in_map_rev(map_rev_attributes, this_val);
}

static void
js_attributes_SetOpaque(JSValueConst this_val, void *node)
{
	REF_JS(this_val);

	if (!node) {
		attr_erase_from_map_rev(map_rev_attributes, this_val);
	} else {
		attr_save_in_map_rev(map_rev_attributes, this_val, node);
	}
}

static void
js_attributes_set_items(JSContext *ctx, JSValue this_val, void *node)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);
	dom_exception err;
	dom_namednodemap *attrs = (dom_namednodemap *)(node);
	unsigned long idx;
	uint32_t num_attrs;

	if (!attrs) {
		return;
	}

	err = dom_namednodemap_get_length(attrs, &num_attrs);
	if (err != DOM_NO_ERR) {
		//dom_namednodemap_unref(attrs);
		return;
	}

	for (idx = 0; idx < num_attrs; ++idx) {
		dom_attr *attr;
		dom_string *name = NULL;

		err = dom_namednodemap_item(attrs, idx, (void *)&attr);

		if (err != DOM_NO_ERR || !attr) {
			continue;
		}
		JSValue obj = getAttr(ctx, attr);

		REF_JS(obj);

		JS_SetPropertyUint32(ctx, this_val, idx, JS_DupValue(ctx, obj));

		err = dom_attr_get_name(attr, &name);

		if (err != DOM_NO_ERR) {
			goto next;
		}

		if (name && !dom_string_caseless_lwc_isequal(name, corestring_lwc_item) && !dom_string_caseless_lwc_isequal(name, corestring_lwc_nameditem)) {
			JS_DefinePropertyValueStr(ctx, this_val, dom_string_data(name), JS_DupValue(ctx, obj), 0);
		}
next:
		if (name) {
			dom_string_unref(name);
		}
		dom_node_unref(attr);
		JS_FreeValue(ctx, obj);
	}
}

static JSValue
js_attributes_get_property_length(JSContext *ctx, JSValueConst this_val)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);
	dom_exception err;
	dom_namednodemap *attrs;
	uint32_t num_attrs;

	struct ecmascript_interpreter *interpreter = (struct ecmascript_interpreter *)JS_GetContextOpaque(ctx);
	struct view_state *vs = interpreter->vs;

	if (!vs) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return JS_EXCEPTION;
	}
	attrs = (dom_namednodemap *)(js_attributes_GetOpaque(this_val));

	if (!attrs) {
		return JS_NewInt32(ctx, 0);
	}
	err = dom_namednodemap_get_length(attrs, &num_attrs);
	if (err != DOM_NO_ERR) {
		//dom_namednodemap_unref(attrs);
		return JS_NewInt32(ctx, 0);
	}

	return JS_NewInt32(ctx, num_attrs);
}

static JSValue
js_attributes_item2(JSContext *ctx, JSValueConst this_val, int idx)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);
	dom_exception err;
	dom_namednodemap *attrs;
	dom_attr *attr;
	JSValue ret;

	attrs = (dom_namednodemap *)(js_attributes_GetOpaque(this_val));

	if (!attrs) {
		return JS_UNDEFINED;
	}

	err = dom_namednodemap_item(attrs, idx, (void *)&attr);

	if (err != DOM_NO_ERR) {
		return JS_UNDEFINED;
	}
	ret = getAttr(ctx, attr);
	dom_node_unref(attr);

	return ret;
}

static JSValue
js_attributes_item(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	if (argc != 1) {
		return JS_UNDEFINED;
	}

	int index;
	JS_ToInt32(ctx, &index, argv[0]);

	return js_attributes_item2(ctx, this_val, index);
}

static JSValue
js_attributes_namedItem2(JSContext *ctx, JSValueConst this_val, const char *str)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	dom_exception err;
	dom_namednodemap *attrs;
	dom_attr *attr = NULL;
	dom_string *name = NULL;
	JSValue obj;

	REF_JS(this_val);

	attrs = (dom_namednodemap *)(js_attributes_GetOpaque(this_val));

	if (!attrs) {
		return JS_UNDEFINED;
	}
	err = dom_string_create((const uint8_t*)str, strlen(str), &name);

	if (err != DOM_NO_ERR) {
		return JS_UNDEFINED;
	}
	err = dom_namednodemap_get_named_item(attrs, name, &attr);
	dom_string_unref(name);

	if (err != DOM_NO_ERR || !attr) {
		return JS_UNDEFINED;
	}
	obj = getAttr(ctx, attr);
	dom_node_unref(attr);

	RETURN_JS(obj);
}

static JSValue
js_attributes_getNamedItem(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	if (argc != 1) {
		return JS_UNDEFINED;
	}

	const char *str;
	size_t len;

	str = JS_ToCStringLen(ctx, &len, argv[0]);

	if (!str) {
		return JS_EXCEPTION;
	}

	JSValue ret = js_attributes_namedItem2(ctx, this_val, str);
	JS_FreeCString(ctx, str);

	RETURN_JS(ret);
}

static JSValue
js_attributes_toString(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	return JS_NewString(ctx, "[attributes object]");
}

static const JSCFunctionListEntry js_attributes_proto_funcs[] = {
	JS_CGETSET_DEF("length", js_attributes_get_property_length, NULL),
	JS_CFUNC_DEF("item", 1, js_attributes_item),
	JS_CFUNC_DEF("getNamedItem", 1, js_attributes_getNamedItem),
	JS_CFUNC_DEF("toString", 0, js_attributes_toString)
};

#if 0
static void
js_attributes_finalizer(JSRuntime *rt, JSValue val)
{
	void *node = js_attributes_GetOpaque(val);

	js_attributes_SetOpaque(val, nullptr);
	map_attributes.erase(node);
}

static JSClassDef js_attributes_class = {
	"attributes",
	js_attributes_finalizer
};

#endif

JSValue
getAttributes(JSContext *ctx, void *node)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	JSValue second = JS_NULL;
	static int initialized;
	/* create the element class */
	if (!initialized) {
		initialized = 1;
	}
	second = attr_find_in_map(map_attributes, node);

	if (!JS_IsNull(second)) {
		JSValue r = JS_DupValue(ctx, second);

		RETURN_JS(r);
	}
	JSValue attributes_obj = JS_NewArray(ctx);
	JS_SetPropertyFunctionList(ctx, attributes_obj, js_attributes_proto_funcs, countof(js_attributes_proto_funcs));

	js_attributes_SetOpaque(attributes_obj, node);
	js_attributes_set_items(ctx, attributes_obj, node);
	attr_save_in_map(map_attributes, node, attributes_obj);

	JSValue rr = JS_DupValue(ctx, attributes_obj);

	RETURN_JS(rr);
}
