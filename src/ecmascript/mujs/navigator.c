/* The mujs navigator object implementation. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elinks.h"

#include "ecmascript/ecmascript.h"
#include "ecmascript/mujs.h"
#include "ecmascript/mujs/navigator.h"
#include "intl/libintl.h"
#include "osdep/sysname.h"
#include "protocol/http/http.h"
#include "util/conv.h"

static void
mjs_navigator_get_property_appCodeName(js_State *J)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	js_pushstring(J, "Mozilla");
}

static void
mjs_navigator_get_property_appName(js_State *J)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	js_pushstring(J, "ELinks (roughly compatible with Netscape Navigator, Mozilla and Microsoft Internet Explorer)");
}

static void
mjs_navigator_get_property_appVersion(js_State *J)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	js_pushstring(J, VERSION);
}

static void
mjs_navigator_get_property_language(js_State *J)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
#ifdef CONFIG_NLS
	if (get_opt_bool("protocol.http.accept_ui_language", NULL)) {
		js_pushstring(J, language_to_iso639(current_language));
		return;
	}
#endif
	js_pushundefined(J);
}

static void
mjs_navigator_get_property_platform(js_State *J)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	js_pushstring(J, system_name);
}

static void
mjs_navigator_get_property_userAgent(js_State *J)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	char *optstr;

	optstr = get_opt_str("protocol.http.user_agent", NULL);

	if (*optstr && strcmp(optstr, " ")) {
		char *ustr, ts[64] = "";
		static char custr[256];
		/* TODO: Somehow get the terminal in which the
		 * document is actually being displayed.  */
		struct terminal *term = get_default_terminal();

		if (term) {
			unsigned int tslen = 0;

			ulongcat(ts, &tslen, term->width, 3, 0);
			ts[tslen++] = 'x';
			ulongcat(ts, &tslen, term->height, 3, 0);
		}
		ustr = subst_user_agent(optstr, VERSION_STRING, system_name, ts);

		if (ustr) {
			safe_strncpy(custr, ustr, 256);
			mem_free(ustr);

			js_pushstring(J, custr);
			return;
		}
	}
	js_pushstring(J, system_name);
}

static void
mjs_navigator_toString(js_State *J)
{
#ifdef ECMASCRIPT_DEBUG
	fprintf(stderr, "%s:%s\n", __FILE__, __FUNCTION__);
#endif
	js_pushstring(J, "[navigator object]");
}

int
mjs_navigator_init(js_State *J)
{
	js_newobject(J);
	{
		addmethod(J, "navigator.toString", mjs_navigator_toString, 0);
		addproperty(J, "navigator.appCodeName", mjs_navigator_get_property_appCodeName, NULL);
		addproperty(J, "navigator.appName", mjs_navigator_get_property_appName, NULL);
		addproperty(J, "navigator.appVersion", mjs_navigator_get_property_appVersion, NULL);
		addproperty(J, "navigator.language", mjs_navigator_get_property_language, NULL);
		addproperty(J, "navigator.platform", mjs_navigator_get_property_platform, NULL);
		addproperty(J, "navigator.userAgent", mjs_navigator_get_property_userAgent, NULL);
	}
	js_defglobal(J, "navigator", JS_DONTENUM);

	return 0;
}
