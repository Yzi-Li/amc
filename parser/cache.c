#include "include/cache.h"
#include "../include/parser.h"
#include "../utils/str/str.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(__unix__)
#include <sys/stat.h>
#elif defined(__WIN32)
#include <direct.h>
#define mkdir(pathname, ...) _mkdir(pathname)
#endif

#define cache_dir_name "amc"
#define cache_dir_path ".cache"

int cache_dir_create(str *root_dir)
{
	str dir = {};
	if (global_parser.target_path.s != NULL)
		return 0;
	str_expand(&dir, root_dir->len
			+ strlen(cache_dir_name)
			+ strlen(cache_dir_path)
			+ 3);
	snprintf(dir.s, dir.len, "%s/%s", root_dir->s, cache_dir_path);
	if (mkdir(dir.s, S_IRWXU | S_IROTH | S_IXOTH) && errno != EEXIST)
		return 1;
	snprintf(dir.s, dir.len, "%s/%s/%s", root_dir->s,
			cache_dir_path,
			cache_dir_name);
	if (mkdir(dir.s, S_IRWXU | S_IROTH | S_IXOTH) && errno != EEXIST)
		return 1;
	global_parser.target_path.s = dir.s;
	global_parser.target_path.len = strlen(dir.s);
	return 0;
}
