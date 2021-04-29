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
#define MAXTHREADS 100

pthread_t tids[MAXTHREADS];

int thread_count = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* #endregion variables/const */

/* #region structs ------------------------------------------------- */

typedef struct mp3info {
	int *filename;
	char *fullpath;
	void *data;
} mp3info_t;

mp3info_t **mp3_index;
int mp3_index_length = 0;

typedef struct dir {
	struct dirent *dir_entry;
	char *path;
} dir_t;

typedef struct dirarr {
	dir_t **arr;
	int *length;
} dirarr_t;

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

/* return int value of a filename */
int get_int(char *filename){
    char *file_str = strdup(filename); // dup filename for strsep
    char *free_file_str = file_str; // get pointer to beginning of file_str

    int file_int = atoi(strsep(&file_str, ".")); // sep and get int value

	free(free_file_str); // free memory

	return file_int;
}

/* concatenate filepath/filename */
char *catpath(char *filename, const char *filepath){
    char buf[MAXNAMLEN];

	strcpy(buf, filepath); // copy the filepath into buf 
    strcat(buf, "/"); // cat a slash between path and name
    strcat(buf, filename); // cat the filename

	return strdup(buf);
}

/* #endregion functions */