/* Get home directory */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> /* OS/2 needs this after sys/types.h */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "elinks.h"

#include "config/home.h"
#include "config/options.h"
#include "intl/libintl.h"
#include "main/main.h"
#include "osdep/osdep.h"
#include "util/memory.h"
#include "util/string.h"


int first_use = 0;
static char *xdg_config_home = NULL;

static inline void
strip_trailing_dir_sep(char *path)
{
	int i;

	for (i = strlen(path) - 1; i > 0; i--)
		if (!dir_sep(path[i]))
			break;

	path[i + 1] = 0;
}

static char *
test_confdir(const char *home, const char *path,
	     char *error_message)
{
	struct stat st;
	char *confdir;

	if (!path || !*path) return NULL;

	if (home && *home && !dir_sep(*path))
		confdir = straconcat(home, STRING_DIR_SEP, path,
				     (char *) NULL);
	else
		confdir = stracpy(path);

	if (!confdir) return NULL;

	strip_trailing_dir_sep(confdir);

	if (stat(confdir, &st)) {
		if (!mkdir(confdir, 0700)) {
#if 0
		/* I've no idea if following is needed for newly created
		 * directories.  It's bad thing to do it everytime. --pasky */
#ifdef HAVE_CHMOD
			chmod(home_elinks, 0700);
#endif
#endif
			return confdir;
		}

	} else if (S_ISDIR(st.st_mode)) {
		first_use = 0;
		return confdir;
	}

	if (error_message) {
		usrerror(gettext(error_message), path, confdir);
		sleep(3);
	}

	mem_free(confdir);

	return NULL;
}

char *
get_xdg_config_home(void)
{
	return xdg_config_home;
}

static char *
get_xdg_config_home_internal(void)
{
	char *config_dir = NULL;
	char *elinks_confdir;
	char *pa;
	char *g_xdg_config_home;
	char *home;

	if (xdg_config_home) {
		return xdg_config_home;
	}
	elinks_confdir = getenv("ELINKS_CONFDIR");
	pa = get_cmd_opt_str("config-dir");

	if (elinks_confdir && *elinks_confdir && (!pa || !*pa)) {
		xdg_config_home = test_confdir(NULL, elinks_confdir, NULL);

		if (xdg_config_home) goto end;
	}
	g_xdg_config_home = getenv("XDG_CONFIG_HOME");

	if (g_xdg_config_home && *g_xdg_config_home) {
		xdg_config_home = test_confdir(g_xdg_config_home,
						pa,
						N_("Commandline options -config-dir set to %s, "
						"but could not create directory %s."));
		if (xdg_config_home) {
			goto end;
		}
		xdg_config_home = test_confdir(g_xdg_config_home, "elinks", NULL);

		if (xdg_config_home) {
			goto end;
		}

		return NULL;
	}
	home = getenv("HOME");

	if (!home || !*home) {
		return NULL;
	}
	config_dir = straconcat(home, STRING_DIR_SEP, ".config", NULL);

	if (!config_dir) {
		return NULL;
	}
	xdg_config_home = test_confdir(config_dir,
				pa,
				N_("Commandline options -config-dir set to %s, "
				"but could not create directory %s."));
	if (xdg_config_home) {
		goto end;
	}
	xdg_config_home = test_confdir(config_dir, "elinks", NULL);

end:
	mem_free_if(config_dir);

	if (xdg_config_home) {
		add_to_strn(&xdg_config_home, STRING_DIR_SEP);
		return xdg_config_home;
	}

	return NULL;
}


#if 0
/* TODO: Check possibility to use <libgen.h> dirname. */
static char *
elinks_dirname(char *path)
{
	int i;
	char *dir;

	if (!path) return NULL;

	dir = stracpy(path);
	if (!dir) return NULL;

	for (i = strlen(dir) - 1; i >= 0; i--)
		if (dir_sep(dir[i]))
			break;

	dir[i + 1] = 0;

	return dir;
}

static char *
get_home(void)
{
	char *home_elinks;
	char *envhome = getenv("HOME_ETC") ?: getenv("HOME");
	char *home = NULL;

	if (!home && envhome)
		home = stracpy(envhome);
	if (!home)
		home = user_appdata_directory();
	if (!home)
		home = elinks_dirname(program.path);

	if (home)
		strip_trailing_dir_sep(home);

	home_elinks = test_confdir(home,
				   get_cmd_opt_str("config-dir"),
				   N_("Commandline options -config-dir set to %s, "
				      "but could not create directory %s."));
	if (home_elinks) goto end;

	home_elinks = test_confdir(home, getenv("ELINKS_CONFDIR"),
				   N_("ELINKS_CONFDIR set to %s, "
				      "but could not create directory %s."));
	if (home_elinks) goto end;

	home_elinks = test_confdir(home, ".elinks", NULL);
	if (home_elinks) goto end;

	home_elinks = test_confdir(home, "elinks", NULL);

end:
	if (home_elinks)
		add_to_strn(&home_elinks, STRING_DIR_SEP);
	mem_free_if(home);

	return home_elinks;
}
#endif

void
init_home(void)
{
	first_use = 1;
	xdg_config_home = get_xdg_config_home_internal();
	if (!xdg_config_home) {
		ERROR(gettext("Unable to find or create ELinks config "
		      "directory. Please check if you have $HOME "
		      "variable set correctly and if you have "
		      "write permission to your home directory."));
		sleep(3);
		return;
	}
}

void
done_home(void)
{
	mem_free_set(&xdg_config_home, NULL);
}
