#ifndef EL__ECMASCRIPT_ECMASCRIPT_H
#define EL__ECMASCRIPT_ECMASCRIPT_H

/* This is a trivial ECMAScript driver. All your base are belong to pasky. */
/* In the future you will get DOM, a complete ECMAScript interface and free
 * plasm displays for everyone. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef CONFIG_ECMASCRIPT_SMJS
#include <jsapi.h>
#endif

#ifdef CONFIG_QUICKJS
#include <quickjs/quickjs.h>
#endif

#ifdef CONFIG_MUJS
#include <mujs.h>
#endif

#ifdef CONFIG_ECMASCRIPT

#include "main/module.h"
#include "main/timer.h"
#include "util/time.h"

//#define ECMASCRIPT_DEBUG 1

#ifdef ECMASCRIPT_DEBUG
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct document_view;
struct form_state;
struct form_view;
struct string;
struct terminal;
struct uri;
struct view_state;

struct ecmascript_string_list_item {
	LIST_HEAD(struct ecmascript_string_list_item);
	struct string string;
	int element_offset;
};

struct ecmascript_interpreter {
	struct view_state *vs;
	void *backend_data;

	/* Nesting level of calls to backend functions.  When this is
	 * nonzero, there are references to backend_data in the C
	 * stack, so it is not safe to free the data yet.  */
	int backend_nesting;

	/* Used by document.write() */
	struct string *ret;

	/* The code evaluated by setTimeout() */
	struct string code;

	/* document.write buffer */
	struct string writecode;

	struct heartbeat *heartbeat;

	/* This is a cross-rerenderings accumulator of
	 * @document.onload_snippets (see its description for juicy details).
	 * They enter this list as they continue to appear there, and they
	 * never leave it (so that we can always find from where to look for
	 * any new snippets in document.onload_snippets). Instead, as we
	 * go through the list we maintain a pointer to the last processed
	 * entry. */
	LIST_OF(struct ecmascript_string_list_item) onload_snippets;
	struct ecmascript_string_list_item *current_onload_snippet;

	/* ID of the {struct document} where those onload_snippets belong to.
	 * It is kept at 0 until it is definitively hard-attached to a given
	 * final document. Then if we suddenly appear with this structure upon
	 * a document with a different ID, we reset the state and start with a
	 * fresh one (normally, that does not happen since reloading sets
	 * ecmascript_fragile, but it can happen i.e. when the urrent document
	 * is reloaded in another tab and then you just cause the current tab
	 * to redraw. */
	unsigned int onload_snippets_cache_id;
	void *ac;
	void *ac2;
#ifdef CONFIG_QUICKJS
	JSValue document_obj;
	JSValue location_obj;
#else
	void *document_obj;
	void *location_obj;
#endif
#ifdef CONFIG_QUICKJS
	JSValueConst fun;
#endif
#ifdef CONFIG_ECMASCRIPT_SMJS
	JS::RootedValue fun;
#endif
#ifdef CONFIG_MUJS
	const char *fun;
#endif
	bool changed;
	int element_offset;
};

struct ecmascript_timeout {
	LIST_HEAD(struct ecmascript_timeout);
	struct string code;
#ifdef CONFIG_QUICKJS
	JSValueConst fun;
#endif
#ifdef CONFIG_ECMASCRIPT_SMJS
	JS::RootedValue fun;
#endif
#ifdef CONFIG_MUJS
	js_State *ctx;
	const char *fun;
#endif
	struct ecmascript_interpreter *interpreter;
	timer_id_T tid;
};


struct delayed_goto {
	/* It might look more convenient to pass doc_view around but it could
	 * disappear during wild dances inside of frames or so. */
	struct view_state *vs;
	struct uri *uri;
};

/* Why is the interpreter bound to {struct view_state} instead of {struct
 * document}? That's easy, because the script won't raid just inside of the
 * document, but it will also want to generate pop-up boxes, adjust form
 * contents (which is doc_view-specific) etc. Of course the cons are that we
 * need to wait with any javascript code execution until we get bound to the
 * view_state through document_view - that means we are going to re-render the
 * document if it contains a <script> area full of document.write()s. And why
 * not bound the interpreter to {struct document_view} then? Because it is
 * reset for each rerendering, and it sucks to do all the magic to preserve the
 * interpreter over the rerenderings (we tried). */

int ecmascript_check_url(char *url, char *frame);
void ecmascript_free_urls(struct module *module);

struct ecmascript_interpreter *ecmascript_get_interpreter(struct view_state*vs);
void ecmascript_put_interpreter(struct ecmascript_interpreter *interpreter);
int ecmascript_get_interpreter_count(void);

void ecmascript_detach_form_view(struct form_view *fv);
void ecmascript_detach_form_state(struct form_state *fs);
void ecmascript_moved_form_state(struct form_state *fs);

void ecmascript_reset_state(struct view_state *vs);

void ecmascript_eval(struct ecmascript_interpreter *interpreter, struct string *code, struct string *ret, int element_offset);
char *ecmascript_eval_stringback(struct ecmascript_interpreter *interpreter, struct string *code);
/* Returns -1 if undefined. */
int ecmascript_eval_boolback(struct ecmascript_interpreter *interpreter, struct string *code);

/* Takes line with the syntax javascript:<ecmascript code>. Activated when user
 * follows a link with this synstax. */
void ecmascript_protocol_handler(struct session *ses, struct uri *uri);

void ecmascript_timeout_dialog(struct terminal *term, int max_exec_time);

void ecmascript_set_action(char **action, char *string);

timer_id_T ecmascript_set_timeout(struct ecmascript_interpreter *interpreter, char *code, int timeout);

#ifdef CONFIG_ECMASCRIPT_SMJS
timer_id_T ecmascript_set_timeout2(struct ecmascript_interpreter *interpreter, JS::HandleValue f, int timeout);
#endif

#ifdef CONFIG_QUICKJS
timer_id_T ecmascript_set_timeout2q(struct ecmascript_interpreter *interpreter, JSValue f, int timeout);
#endif

#ifdef CONFIG_MUJS
timer_id_T ecmascript_set_timeout2m(js_State *J, const char *handle, int timeout);
#endif

int get_ecmascript_enable(struct ecmascript_interpreter *interpreter);

void check_for_rerender(struct ecmascript_interpreter *interpreter, const char* text);

void toggle_ecmascript(struct session *ses);

void *document_parse(struct document *document);
void free_document(void *doc);
void location_goto(struct document_view *doc_view, char *url);
void location_goto_const(struct document_view *doc_view, const char *url);

struct string *add_to_ecmascript_string_list(LIST_OF(struct ecmascript_string_list_item) *list, const char *string, int length, int element_offset);

void free_ecmascript_string_list(LIST_OF(struct ecmascript_string_list_item) *list);

extern char *console_error_filename;
extern char *console_log_filename;

extern char *local_storage_filename;
extern int local_storage_ready;

extern struct module ecmascript_module;

#ifdef __cplusplus
}
#endif

#endif

#endif
