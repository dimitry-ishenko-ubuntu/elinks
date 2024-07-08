/* The QuickJS console object implementation. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elinks.h"

#include "ecmascript/ecmascript.h"
#include "ecmascript/quickjs.h"
#include "ecmascript/quickjs/console.h"

#define DEBUG 0

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static JSClassID js_console_class_id;

static JSValue
js_console_log_common(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, const char *log_filename)
{
	REF_JS(this_val);

	if (argc != 1 || !log_filename)
	{
		return JS_UNDEFINED;
	}

	if (get_opt_bool("ecmascript.enable_console_log", NULL))
	{
		size_t len;

		const char *str = JS_ToCStringLen(ctx, &len, argv[0]);

		if (!str) {
			return JS_EXCEPTION;
		}
		FILE *f = fopen(log_filename, "a");

		if (f)
		{
			fprintf(f, "%s\n", str);
			fclose(f);
		}
		JS_FreeCString(ctx, str);
	}

	return JS_UNDEFINED;
}

static JSValue
js_console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	return js_console_log_common(ctx, this_val, argc, argv, console_log_filename);
}

static JSValue
js_console_error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	return js_console_log_common(ctx, this_val, argc, argv, console_error_filename);
}

static JSValue
js_console_toString(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	REF_JS(this_val);

	return JS_NewString(ctx, "[console object]");
}

static const JSCFunctionListEntry js_console_funcs[] = {
	JS_CFUNC_DEF("log", 1, js_console_log),
	JS_CFUNC_DEF("error", 1, js_console_error),
	JS_CFUNC_DEF("toString", 0, js_console_toString)
};

static JSClassDef js_console_class = {
	"console",
};

int
js_console_init(JSContext *ctx)
{
	JSValue console_proto;

	/* create the console class */
	JS_NewClassID(&js_console_class_id);
	JS_NewClass(JS_GetRuntime(ctx), js_console_class_id, &js_console_class);

	JSValue global_obj = JS_GetGlobalObject(ctx);
	REF_JS(global_obj);

	console_proto = JS_NewObject(ctx);
	REF_JS(console_proto);

	JS_SetPropertyFunctionList(ctx, console_proto, js_console_funcs, countof(js_console_funcs));
	JS_SetClassProto(ctx, js_console_class_id, console_proto);
	JS_SetPropertyStr(ctx, global_obj, "console", JS_DupValue(ctx, console_proto));

	JS_FreeValue(ctx, global_obj);

	return 0;
}
