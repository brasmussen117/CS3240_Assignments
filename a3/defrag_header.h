/* #region includes ------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <dirent.h>
#include <stdbool.h>
#include <pthread.h>
/* #endregion includes */

/* #region global variables/const ---------------------------------- */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* mp3 array */
char **mp3_entries;
int *mp3_length;

char buf[MAXNAMLEN];
/* #endregion variables/const */

/* #region structs ------------------------------------------------- */
typedef struct index
{
	char **entries;
	int *length;
} index_t;
/* #endregion structs */

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
	return ((strcmp(get_filename_ext(filename), "mp3") == 0) ? true : false);
}
/* #endregion functions */