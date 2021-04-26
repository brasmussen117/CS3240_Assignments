#include "defrag_header.h"

/* function prototypes --------------------------------------------- */
void *searchdir(void *);
void makeindex(char *, DIR *);

int main(int argc, char const *argv[])
{
    /* check for necessary arguments ------------------------------- */
    if (argc < 3) // check for minimum args
    {
        fprintf(stderr, "Usage: <dir to defrag> <output filename>\n");
        return 1;
    }
    if (argc > 3) // check that no extra args given
    {
        errno = E2BIG;
        fprintf(stderr, "./defrag: Arg list too long\n");
        return 1;
    }

    /* open file --------------------------------------------------- */
    const char *top_name = argv[1];

    DIR *top_dir = opendir(top_name); // open the file given as argument in command line

    if (top_dir == NULL) // check that fopen was sucessful
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "./defrag: cannot open(\"%s\"): No such file or directory\n", top_name);
        }
        else
        {
			perror("./defrag: cannot open directory");
        }
        return 1;
    }

    /* prep -------------------------------------------------------- */
    mp3_entries = NULL;
    mp3_length = malloc(sizeof(int));
    *mp3_length = 0;
    
    struct dirent *dir_entry = NULL;
    DIR *next_dir = NULL;

    // int err;
	
	/* top loop ---------------------------------------------------- */

	while ((dir_entry = readdir(top_dir)))
	{
        if ((strcmp(dir_entry->d_name, ".")==0) || // check if self or parent pointers
            (strcmp(dir_entry->d_name, "..")==0))
        {
            continue; // skip self or parent pointers
        }

		if (dir_entry->d_type == DT_DIR) // check if dir_entry entry is subdir
		{
            if (!(next_dir = opendir(dir_entry->d_name))) // check if next_dir can be made
            {
                perror("ERROR: main: could not open next_dir");
                exit(EXIT_FAILURE);
            }

            // TODO: handle it
            // err = pthread_create((pthread_t)dir_entry->d_ino, NULL, searchdir, next_dir);
		} else if (is_mp3(dir_entry->d_name)) // check if file ext is mp3
		{ 
			makeindex(dir_entry->d_name);
			// handle the file

		} // else file was other type not used here, skip it
	}

    if (closedir(top_dir) != 0)
    {
        perror("ERROR: main: failed to close top_dir");
        exit(EXIT_FAILURE);
    }

	return 0;
}

/* thread func to search a dir for mp3 files */
// void *searchdir(void *arg){
//     DIR *current = (DIR *)arg;


//     /* 
//     loop through each entry
//         if entry is subdir, call self
//         if entry is mp3, call makeindex
//      */
// }

/* make an index entry in mp3_entries from filepath/filename */
void makeindex(char *filename){
    


//     strcpy(buf, "./");
//     char *filepath = realpath(strcat(buf, filename), NULL);

//     if (filepath == NULL)
//     {
//         perror("ERROR: makeindex: failed to get path");
//         exit(EXIT_FAILURE);
//     }
        
    char *file_str = strdup(filename); // dup filename for strsep
    char *free_file_str = file_str;

    int file_int = atoi(strsep(&file_str, ".")); // sep and get int value

    if (*mp3_length < (file_int + 1)) // check that mp3_entries is large enough
    {
        mp3_entries = realloc(mp3_entries, (sizeof(char *)*(file_int + 1))); // realloc big enoug
        if (mp3_entries == NULL) // check that realloc was successful
        {
            perror("ERROR: makeindex: failed to realloc mp3_entries");
            exit(EXIT_FAILURE);
        }
    }
    
//     // strcpy(buf, filepath); // copy the filepath into buf
//     // strcat(buf, "/"); // cat a slash between path and name
//     // strcat(buf, filename); // cat the filename

    mp3_entries[file_int] = strdup(filepath); // dup entry into array
    *mp3_length = *mp3_length + 1; // inc mp3_length
    
    free(free_file_str); // free memory
}