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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* #region global variables/const ---------------------------------- */

#define ERROR (-1)

#define DEFAULTOUTFN "./debug/myout.mp3"

/* #endregion global variables/const */

/* #region testing vars -------------------------------------------- */

const char *testmp3paths[] = { 
	"starters/dirs/home/etc/etc/0.mp3",
	"starters/dirs/bin/bin/dev/1.mp3",
	"starters/dirs/home/home/proc/2.mp3",
	"starters/dirs/usr/home/3.mp3",
	"starters/dirs/dev/proc/home/4.mp3",
	"starters/dirs/var/proc/dev/5.mp3",
	"starters/dirs/dev/home/usr/6.mp3",
	"starters/dirs/etc/dev/usr/7.mp3"
};// TODO: remove debug

/* #endregion */

/* #region structs ------------------------------------------------- */

typedef struct mp3info
{
	int filename;
	const char *path;
	// void *data;
} mp3info_t;

mp3info_t **mp3_index; // global mp3info array
int mp3_index_length = 0; // global length of global mp3info array

typedef struct dir
{
	DIR *dir;
	const char *path;
} dir_t;

/* #endregion structs */

/* #region function prototypes ------------------------------------- */

void *searchdir(void *);
void makeindex(struct dirent *, const char *);
mp3info_t **testmp3s();
// void *mp3merge(mp3info_t **, int); TODO: remove
void catmp3(mp3info_t **, int , char *);
static void catpipe(char ***, char *);

/* #endregion */

/* #region functions ----------------------------------------------- */

/* return extension from filename string
	return pointer to extension if present

	found on Stack Overflow: https://stackoverflow.com/a/5309508/13305483
	author: https://stackoverflow.com/users/298479/thiefmaster
*/
const char *get_filename_ext(const char *filename)
{
	const char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}

/* check if filename string contains mp3 extension */
bool is_mp3(const char *filename)
{
	return ((strcmp(get_filename_ext(filename), "mp3") == 0) ? true : false);
}

/* return int value of a filename */
int get_int(char *filename)
{
	char *file_str = strdup(filename); // dup filename for strsep
	char *free_file_str = file_str;	   // get pointer to beginning of file_str

	int file_int = atoi(strsep(&file_str, ".")); // sep and get int value

	free(free_file_str); // free memory

	return file_int;
}

/* concatenate filepath/filename
	__memory allocated, must be freed__
*/
char *catpath(char *filename, const char *filepath)
{
	char buf[MAXNAMLEN];

	strcpy(buf, filepath); // copy the filepath into buf
	strcat(buf, "/");	   // cat a slash between path and name
	strcat(buf, filename); // cat the filename

	return strdup(buf);
}

/* get the size in bytes of target file */
off_t getfilesize(const char *target_path)
{
    struct stat target_stat;

    if (stat(target_path, &target_stat) == ERROR)
    {
        perror("getfilesize: stat failed");
        exit(EXIT_FAILURE);
    }
    
    return target_stat.st_size;
}

/* make hard-coded mp3 data structure for testing
    return array of mp3info_t*
    ** memory is allocated and must be freed **
*/
mp3info_t **testmp3s()
{
    mp3info_t **output = malloc(sizeof(mp3info_t *) * 8); // 8 files in dirs starter file

    for (int i = 0; i < 8; i++)
    {
        output[i] = malloc(sizeof(mp3info_t));

        output[i]->filename = i;

        output[i]->path = strdup(testmp3paths[i]);

        // /* open file contents -------------------------------------- */
        // FILE *contents = fopen(output[i]->path, "r"); TODO: remove

        // if (contents == NULL) // check that fopen was sucessful
        // {
        //     if (errno == ENOENT)
        //     {
        //         fprintf(stderr, "testmp3s: cannot open(\"%s\"): No such file or directory\n", output[i]->path);
        //     }
        //     else
        //     {
        //         perror("testmp3s: cannot open contents");
        //     }
        //     exit(EXIT_FAILURE);
        // }

        // /* get size of contents ------------------------------------ */
        // off_t contents_size = getfilesize(output[i]->path);

        // /* malloc and read contents into mp3_index ---------------------- */

        // output[i]->data = malloc(contents_size); // malloc space for newest entry

        // if (fread(output[i]->data, 1, contents_size, contents) 
        //     != contents_size)
        // {
        //     perror("testmp3s: cannot read file contents");
        //     exit(EXIT_FAILURE);
        // }

        mp3_index_length++; // inc mp3_index_length
        
        /* close file -------------------------------------------------- */
        // if (fclose(contents) != 0) // check file close success TODO: remove
        // {
        //     fprintf(stderr, "testmp3s: file not successfully closed: (\"%s\")\n", output[i]->path);
        //     exit(EXIT_FAILURE);
        // }
        
    }
    
    return output;
}

/* #endregion functions */