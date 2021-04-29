#include "defrag_header.h"

/* function prototypes --------------------------------------------- */
void *searchdir(void *);
void makeindex(struct dirent *, const char *);

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
    // dirarr_t *dir_arr = malloc(sizeof(dirarr_t));
    // dir_arr->arr = NULL;
    // dir_arr->length = malloc(sizeof(int));
    // dir_arr->length = 0;

    struct dirent *dir_entry = NULL;
    dir_t *next_dir = malloc(sizeof(dir_t));

    int err; // TODO: finish this
	
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
            next_dir->dir_entry = dir_entry;
            next_dir->path = catpath(dir_entry->d_name, top_name);
            fprintf(stdout, "main: opened subdir (\"%s\")", dir_entry->d_name);
            // TODO: handle it
            err = pthread_create(&tids[thread_count++], NULL, searchdir, next_dir);
            if (err != 0) 
            {
                perror("ERROR: main: could not create thread");
                exit(EXIT_FAILURE);
            }
		} else if (is_mp3(dir_entry->d_name)) // check if file ext is mp3
		{ 
			makeindex(dir_entry, top_name);
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
void *searchdir(void *arg){
    // DIR *current = (DIR *)arg;

    fprintf(stdout, "searchdir: ");

    /* 
    loop through each entry
        if entry is subdir, call self
        if entry is mp3, call makeindex
     */
    return arg;
}

/* make an index entry in mp3_entries from filepath/filename 
    TODO: fix description
*/
void makeindex(struct dirent *entry, const char *filepath){
    /* get int value of filename ----------------------------------- */
    int file_int = get_int(entry->d_name);

    /* check mp3_index size, realloc as needed --------------------- */
    if (mp3_index_length < (file_int + 1)) // check that mp3_index is large enough
    {
        mp3_index = realloc(mp3_index, (sizeof(void *)*(file_int + 1))); // realloc big enoug
        if (mp3_index == NULL) // check that realloc was successful
        {
            perror("ERROR: makeindex: failed to realloc mp3_index");
            exit(EXIT_FAILURE);
        }
    }

    /* put filename to index --------------------------------------- */
    *mp3_index[file_int]->filename = file_int;

    /* get full path name and put to index ------------------------- */
    mp3_index[file_int]->fullpath = catpath(entry->d_name, filepath);

    /* open file contents ------------------------------------------ */
    FILE *contents = fopen(mp3_index[file_int]->fullpath, O_RDONLY);

    if (contents == NULL) // check that fopen was sucessful
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "ERROR: makeindex: cannot open(\"%s\"): No such file or directory\n", entry->d_name);
        }
        else
        {
            perror("ERROR: makeindex: cannot open contents");
        }
        exit(EXIT_FAILURE);
    }

    /* malloc and read contents into mp3_index ---------------------- */

    mp3_index[file_int]->data = malloc(entry->d_reclen); // malloc space for newest entry

    if (fread(mp3_index[file_int]->data, 1, entry->d_reclen, contents) 
        != entry->d_reclen)
    {
        perror("ERROR: makeindex: cannot read file contents");
        exit(EXIT_FAILURE);
    }

    mp3_index_length++; // inc mp3_index_length
    
    /* close file -------------------------------------------------- */
    if (fclose(contents) != 0) // check file close success
    {
        fprintf(stderr, "parser::main: file not successfully closed: (\"%s\")\n", entry->d_name);
        exit(EXIT_FAILURE);
    }
}
