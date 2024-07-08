#ifndef EL__TERMINAL_SIXEL_H
#define EL__TERMINAL_SIXEL_H

#include <sixel.h>
#include "util/lists.h"
#include "util/string.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_LIBSIXEL
struct document;
struct terminal;


struct image {
	LIST_HEAD_EL(struct image);
	struct string pixels;
	int x;
	int y;
	int width;
	int height;
};

void delete_image(struct image *im);

void try_to_draw_images(struct terminal *term);

/* return height of image in terminal rows */
int add_image_to_document(struct document *doc, struct string *pixels, int lineno);

struct image *copy_frame(struct image *src, int box_width, int box_height, int cell_width, int cell_height, int dx, int dy);


#endif

#ifdef __cplusplus
}
#endif

#endif /* EL__TERMINAL_SIXEL_H */
