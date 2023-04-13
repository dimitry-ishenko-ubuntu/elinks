#ifndef EL__OSDEP_DOS_DOS_H
#define EL__OSDEP_DOS_DOS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_OS_DOS


#define DOS_EXTRA_KEYBOARD

#ifdef DOS_EXTRA_KEYBOARD
#define OS_SETRAW
#endif

#define EINTRLOOPX(ret_, call_, x_) \
do {                                \
	(ret_) = (call_); \
} while ((ret_) == (x_) && errno == EINTR)

#define EINTRLOOP(ret_, call_)  EINTRLOOPX(ret_, call_, -1)

#include <sys/types.h>


struct timeval;

int dos_read(int fd, void *buf, size_t size);
int dos_write(int fd, const void *buf, size_t size);
int dos_pipe(int fd[2]);
int dos_close(int fd);
int dos_select(int n, fd_set *rs, fd_set *ws, fd_set *es, struct timeval *t, int from_main_loop);
void save_terminal(void);
void restore_terminal(void);
int dos_setraw(int ctl, int save);
void os_seed_random(unsigned char **pool, int *pool_size);
int os_default_charset(void);

void done_draw(void);
int get_system_env(void);
void get_terminal_size(int fd, int *x, int *y);
void *handle_mouse(int cons, void (*fn)(void *, char *, int), void *data);
void handle_terminal_resize(int fd, void (*fn)(void));
void init_osdep(void);
int is_xterm(void);
void resume_mouse(void *data);
int set_nonblocking_fd(int fd);
void suspend_mouse(void *data);
void terminate_osdep(void);
void unhandle_mouse(void *data);
void unhandle_terminal_resize(int fd);
void want_draw(void);



#ifndef DOS_OVERRIDES_SELF

#define read dos_read
#define write dos_write
#define pipe dos_pipe
#define close dos_close

#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
