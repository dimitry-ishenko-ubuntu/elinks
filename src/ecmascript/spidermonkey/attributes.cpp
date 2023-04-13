/* The SpiderMonkey attributes object implementation. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elinks.h"

#include "ecmascript/spidermonkey/util.h"
#include <jsfriendapi.h>

#include "bfu/dialog.h"
#include "cache/cache.h"
#include "cookies/cookies.h"
#include "dialogs/menu.h"
#include "dialogs/status.h"
#include "document/html/frames.h"
#include "document/document.h"
#include "document/forms.h"
#include "document/view.h"
#include "ecmascript/ecmascript.h"
#include "ecmascript/spidermonkey/attr.h"
#include "ecmascript/spidermonkey/attributes.h"
#include "intl/libintl.h"
#include "main/select.h"
#include "osdep/newwin.h"
#include "osdep/sysname.h"
#include "protocol/http/http.h"
#include "protocol/uri.h"
#include "session/history.h"
#include "session/location.h"
#include "session/session.h"
#include "session/task.h"
#include "terminal/tab.h"
#include "terminal/terminal.h"
#include "util/conv.h"
#include "util/memory.h"
#include "util/string.h"
#include "viewer/text/draw.h"
#include "viewer/text/form.h"
#include "viewer/text/link.h"
#include "viewer/text/vs.h"

#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include <libxml++/libxml++.h>
#include <libxml++/attributenode.h>
#include <libxml++/parsers/domparser.h>

#include <iostream>
#include <algorithm>
#include <string>

static bool attributes_item(JSContext *ctx, unsigned int argc, JS::Value *rval);
static bool attributes_getNamedItem(JSContext *ctx, unsigned int argc, JS::Value *rval);
static bool attributes_item2(JSContext *ctx, JS::HandleObject hobj, int index, JS::MutableHandleValue hvp);
static bool attributes_namedItem2(JSContext *ctx, JS::HandleObject hobj, char *str, JS::MutableHandleValue hvp);

static void attributes_finalize(JS::GCContext *op, JSObject *obj)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
}

JSClassOps attributes_ops = {
	nullptr,  // addProperty
	nullptr,  // deleteProperty
	nullptr,  // enumerate
	nullptr,  // newEnumerate
	nullptr,  // resolve
	nullptr,  // mayResolve
	attributes_finalize, // finalize
	nullptr,  // call
	nullptr,  // construct
	JS_GlobalObjectTraceHook
};

JSClass attributes_class = {
	"attributes",
	JSCLASS_HAS_RESERVED_SLOTS(1),
	&attributes_ops
};

static const spidermonkeyFunctionSpec attributes_funcs[] = {
	{ "item",		attributes_item,		1 },
	{ "getNamedItem",		attributes_getNamedItem,	1 },
	{ NULL }
};

static bool attributes_get_property_length(JSContext *ctx, unsigned int argc, JS::Value *vp);

static JSPropertySpec attributes_props[] = {
	JS_PSG("length",	attributes_get_property_length, JSPROP_ENUMERATE),
	JS_PS_END
};

static bool
attributes_set_items(JSContext *ctx, JS::HandleObject hobj, void *node)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif

	JS::Realm *comp = js::GetContextRealm(ctx);

	if (!comp) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	/* This can be called if @obj if not itself an instance of the
	 * appropriate class but has one in its prototype chain.  Fail
	 * such calls.  */
	if (!JS_InstanceOf(ctx, hobj, &attributes_class, NULL)) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	xmlpp::Element::AttributeList *al = JS::GetMaybePtrFromReservedSlot<xmlpp::Element::AttributeList>(hobj, 0);

	if (!al) {
		return true;
	}

	auto it = al->begin();
	auto end = al->end();
	int i = 0;

	for (;it != end; ++it, ++i) {
		xmlpp::Attribute *attr = *it;

		if (!attr) {
			continue;
		}

		JSObject *obj = getAttr(ctx, attr);

		if (!obj) {
			continue;
		}
		JS::RootedObject v(ctx, obj);
		JS::RootedValue ro(ctx, JS::ObjectOrNullValue(v));
		JS_SetElement(ctx, hobj, i, ro);

		xmlpp::ustring name = attr->get_name();

		if (name != "" && name != "item" && name != "namedItem") {
			JS_DefineProperty(ctx, hobj, name.c_str(), ro, JSPROP_ENUMERATE);
		}
	}

	return true;
}

static bool
attributes_get_property_length(JSContext *ctx, unsigned int argc, JS::Value *vp)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	JS::RootedObject hobj(ctx, &args.thisv().toObject());

	struct view_state *vs;
	JS::Realm *comp = js::GetContextRealm(ctx);

	if (!comp) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	struct ecmascript_interpreter *interpreter = (struct ecmascript_interpreter *)JS::GetRealmPrivate(comp);

	/* This can be called if @obj if not itself an instance of the
	 * appropriate class but has one in its prototype chain.  Fail
	 * such calls.  */
	if (!JS_InstanceOf(ctx, hobj, &attributes_class, NULL)) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	vs = interpreter->vs;
	if (!vs) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	xmlpp::Element::AttributeList *al = JS::GetMaybePtrFromReservedSlot<xmlpp::Element::AttributeList>(hobj, 0);

	if (!al) {
		args.rval().setInt32(0);
		return true;
	}

	args.rval().setInt32(al->size());

	return true;
}

static bool
attributes_item(JSContext *ctx, unsigned int argc, JS::Value *vp)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	JS::Value val;
	JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
	JS::RootedObject hobj(ctx, &args.thisv().toObject());
	JS::RootedValue rval(ctx, val);

	int index = args[0].toInt32();
	bool ret = attributes_item2(ctx, hobj, index, &rval);
	args.rval().set(rval);

	return ret;
}

static bool
attributes_getNamedItem(JSContext *ctx, unsigned int argc, JS::Value *vp)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	JS::Value val;
	JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
	JS::RootedObject hobj(ctx, &args.thisv().toObject());
	JS::RootedValue rval(ctx, val);

	char *str = jsval_to_string(ctx, args[0]);
	bool ret = attributes_namedItem2(ctx, hobj, str, &rval);
	args.rval().set(rval);

	mem_free_if(str);

	return ret;
}

static bool
attributes_item2(JSContext *ctx, JS::HandleObject hobj, int index, JS::MutableHandleValue hvp)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	JS::Realm *comp = js::GetContextRealm(ctx);

	if (!comp) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	if (!JS_InstanceOf(ctx, hobj, &attributes_class, NULL)) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	hvp.setUndefined();

	xmlpp::Element::AttributeList *al = JS::GetMaybePtrFromReservedSlot<xmlpp::Element::AttributeList>(hobj, 0);

	if (!al) {
		return true;
	}

	auto it = al->begin();
	auto end = al->end();
	int i = 0;

	for (;it != end; ++it, ++i) {
		if (i != index) {
			continue;
		}
		xmlpp::Attribute *attr = *it;
		JSObject *obj = getAttr(ctx, attr);
		hvp.setObject(*obj);
		break;
	}

	return true;
}

static bool
attributes_namedItem2(JSContext *ctx, JS::HandleObject hobj, char *str, JS::MutableHandleValue hvp)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	JS::Realm *comp = js::GetContextRealm(ctx);

	if (!comp) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	if (!JS_InstanceOf(ctx, hobj, &attributes_class, NULL)) {
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s %d\n", __FILE__, __FUNCTION__, __LINE__);
#endif
		return false;
	}

	xmlpp::Element::AttributeList *al = JS::GetMaybePtrFromReservedSlot<xmlpp::Element::AttributeList>(hobj, 0);

	hvp.setUndefined();

	if (!al) {
		return true;
	}

	xmlpp::ustring name = str;

	auto it = al->begin();
	auto end = al->end();

	for (; it != end; ++it) {
		auto attr = dynamic_cast<xmlpp::AttributeNode*>(*it);

		if (!attr) {
			continue;
		}

		if (name == attr->get_name()) {
			JSObject *obj = (JSObject *)getAttr(ctx, attr);
			hvp.setObject(*obj);
			return true;
		}
	}

	return true;
}

JSObject *
getAttributes(JSContext *ctx, void *node)
{
	JSObject *el = JS_NewObject(ctx, &attributes_class);

	if (!el) {
		return NULL;
	}

	JS::RootedObject r_el(ctx, el);

	JS_DefineProperties(ctx, r_el, (JSPropertySpec *) attributes_props);
	spidermonkey_DefineFunctions(ctx, el, attributes_funcs);

	JS::SetReservedSlot(el, 0, JS::PrivateValue(node));
	attributes_set_items(ctx, r_el, node);

	return el;
}
