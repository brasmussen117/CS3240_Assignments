#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <dirent.h>
#include <stdbool.h>

/* #region  variables/const ---------------------------------------- */

/* #endregion variables/const */

/* #region functions ----------------------------------------------- */

/* return extension from filename string
	return pointer to extension if present
*/
const char *get_filename_ext(const char *filename)
{
	const char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}
/* 	citation
	found on Stack Overflow: https://stackoverflow.com/a/5309508/13305483
	author: https://stackoverflow.com/users/298479/thiefmaster
 */

/* check if filename string contains mp3 extension */
bool is_mp3(const char *filename)
{
	if (strcmp(get_filename_ext(filename), "mp3") == 0){
		return true;
	} else
	{
		return false;
	}
}
/* #endregion functions */