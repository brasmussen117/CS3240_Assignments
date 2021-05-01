#include "defrag_header.h"

/* function prototypes --------------------------------------------- */
void *searchdir(void *);
void makeindex(struct dirent *, const char *);
mp3info_t **testmp3s();
void *mp3merge(mp3info_t **, int);

int main(int argc, char const *argv[]) // #############################
{
    /* check for necessary arguments ------------------------------- */
    if (argc < 3) // check for minimum args
    {
        fprintf(stderr, "Usage: <dir> <output_filename>\n");
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

    /* spin off new threads ---------------------------------------- */
    dir_t *next_dir = malloc(sizeof(dir_t));

    next_dir->dir = top_dir;
    next_dir->path = top_name;

    pthread_t topid;

    int err = pthread_create(&topid, NULL, searchdir, next_dir);
    
    if (err != 0) 
    {
        perror("ERROR: main: could not create thread");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "main: initialized top thread: %lu, dir\n", topid);

    /* rejoin threads ---------------------------------------------- */
    // for (size_t i = 0; i < thread_count; i++) // TODO: remove
    // {
        pthread_join(topid, NULL);
    // }
    
    /* merge mp3s -------------------------------------------------- */
    void *mergedmp3s = mp3merge(testmp3s(), 8); // TODO: remove debug

    /* create ouput file */
    FILE *output_mp3 = fopen(DEFAULTOUTFN, "w"); // TODO: remove debug

    /* write merged contents to the file */
    if (fwrite(mergedmp3s, sizeof(*mergedmp3s), 1, output_mp3) != sizeof(*mergedmp3s))
    {
        perror("main: failed to write mp3 output file");
        exit(EXIT_FAILURE);
    }

	return 0;
} // main #############################################################

/* thread func to search a dir for mp3 files */
void *searchdir(void *arg){
    dir_t *current = (dir_t *)arg;

    if (current->dir == NULL)
    {
        fprintf(stderr, "searchdir: failed to cast input arg\n");
        exit(EXIT_FAILURE);
    }

    int localthreadcount = 0;
    pthread_t *localtids = NULL;

    // fprintf(stdout, "searchdir: name: %s; type: %i; path: %s\n", current->dir->d_name, current->dir->d_type, current->path); // TODO: remove debug

    /* open file --------------------------------------------------- */
    // DIR *new_dir = opendir(current->path); // open the file given as argument in command line // TODO: remove

    // if (new_dir == NULL) // check that fopen was sucessful
    // {
    //     if (errno == ENOENT)
    //     {
    //         fprintf(stderr, "searchdir: cannot open(\"%s\"): No such file or directory\n", current->path);
    //     }
    //     else
    //     {
	// 		perror("searchdir: cannot open directory");
    //     }
    //     exit(EXIT_FAILURE);
    // }

    /* prep -------------------------------------------------------- */
    struct dirent *dir_entry = NULL;
    dir_t *next_dir = malloc(sizeof(dir_t));

	/* loop -------------------------------------------------------- */

	while ((dir_entry = readdir(current->dir)))
	{
        if ((strcmp(dir_entry->d_name, ".")==0) || // check if self or parent pointers
            (strcmp(dir_entry->d_name, "..")==0))
        {
            continue; // skip self or parent pointers
        }

		if (dir_entry->d_type == DT_DIR) // check if dir_entry entry is subdir
		{
            next_dir->path = catpath(dir_entry->d_name, current->path);
            next_dir->dir = opendir(next_dir->path);
            
            pthread_mutex_lock(&lock); // **************************
            fprintf(stdout, "searchdir: tid: %lu: found subdir (\"%s\")\n", pthread_self(), next_dir->path); // TODO: remove debug
            
            localtids = realloc(localtids, sizeof(pthread_t) * ++localthreadcount);

            int err = pthread_create(&tids[thread_count++], NULL, searchdir, next_dir);
            
            if (err != 0) 
            {
                perror("searchdir: main: could not create thread");
                exit(EXIT_FAILURE);
            }
            pthread_mutex_unlock(&lock); // ************************
		} 
        else if (is_mp3(dir_entry->d_name)) // check if file ext is mp3
		{
            pthread_mutex_lock(&lock);
			makeindex(dir_entry, current->path);
            pthread_mutex_unlock(&lock);
		} // else file was other type not used here, skip it
	}

    for (size_t i = 0; i < localthreadcount; i++)
    {
        pthread_join(localtids[i], NULL);
    }

    return NULL;
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

    /* malloc new index -------------------------------------------- */
    mp3_index[file_int] = malloc(sizeof(mp3info_t));

    /* put filename to index --------------------------------------- */
    mp3_index[file_int]->filename = file_int;

    /* get full path name and put to index ------------------------- */
    mp3_index[file_int]->path = catpath(entry->d_name, filepath);

    /* open file contents ------------------------------------------ */
    FILE *contents = fopen(mp3_index[file_int]->path, "r");

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

        /* open file contents -------------------------------------- */
        FILE *contents = fopen(output[i]->path, "r");

        if (contents == NULL) // check that fopen was sucessful
        {
            if (errno == ENOENT)
            {
                fprintf(stderr, "ERROR: makeindex: cannot open(\"%s\"): No such file or directory\n", output[i]->path);
            }
            else
            {
                perror("ERROR: makeindex: cannot open contents");
            }
            exit(EXIT_FAILURE);
        }

        /* get size of contents ------------------------------------ */
        if (fseek(contents, 0, SEEK_END) == -1)
        {
            perror("testmp3s: fseek failed");
            exit(EXIT_FAILURE);
        }

        long contents_size = ftell(contents);

        if (contents_size == -1)
        {
            perror("testmp3s: ftell failed");
            exit(EXIT_FAILURE);
        }

        rewind(contents);

        /* malloc and read contents into mp3_index ---------------------- */

        output[i]->data = malloc(contents_size); // malloc space for newest entry

        if (fread(output[i]->data, 1, contents_size, contents) 
            != contents_size)
        {
            perror("ERROR: makeindex: cannot read file contents");
            exit(EXIT_FAILURE);
        }

        mp3_index_length++; // inc mp3_index_length
        
        /* close file -------------------------------------------------- */
        if (fclose(contents) != 0) // check file close success
        {
            fprintf(stderr, "parser::main: file not successfully closed: (\"%s\")\n", output[i]->path);
            exit(EXIT_FAILURE);
        }
        
    }
    
    return output;
}

/* merge mp3 file segments into one
    return ptr to merged data
    ** memory is allocated and must be freed **
*/
void *mp3merge(mp3info_t **mp3index, int mp3indexlen)
{
    /* get size of total file -------------------------------------- */
    size_t totalsize = 0; // sum of the sizes of the individual segments
    for (size_t i = 0; i < mp3indexlen; i++) // loop to sum each segment
    {
        totalsize += sizeof(*mp3index[i]->data);
    }
    
    /* setup and append data to output ----------------------------- */
    void *output = malloc(totalsize); // output var, malloc'd to sum of data len
    void *endptr = output; // ptr for appending

    for (size_t i = 0; i < mp3indexlen; i++) // loop to append each segment
    {
        endptr = memcpy(endptr, mp3index[i]->data, sizeof(*mp3index[i]->data)); // copy segment to end of output
        endptr += sizeof(*mp3index[i]->data); // shift endptr past the end of last appended segment
    }
    
    return output; // return ptr to beginning of 
}

/* free functions -------------------------------------------------- */
// TODO: this
