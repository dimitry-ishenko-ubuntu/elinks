#ifndef EL__DOCUMENT_HTML_IFRAMES_H
#define EL__DOCUMENT_HTML_IFRAMES_H

#ifdef __cplusplus
extern "C" {
#endif

struct document_options;
struct el_box;
struct iframeset_desc;

struct iframe_desc {
	char *name;
	struct uri *uri;
	struct el_box box;
	int nlink;
};

struct iframeset_desc {
	int n;

	struct iframe_desc iframe_desc[1]; /* must be last of struct. --Zas */
};

void add_iframeset_entry(struct iframeset_desc **parent, char *url, char *name, int y, int width, int height, int nlink);
void format_iframes(struct session *ses, struct iframeset_desc *ifsd, struct document_options *op, int depth);

#ifdef __cplusplus
}
#endif

#endif
