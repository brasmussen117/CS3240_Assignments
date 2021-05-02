#include "defrag_header.h"

/* function prototypes --------------------------------------------- */
void *searchdir(void *);
void makeindex(struct dirent *, const char *);
mp3info_t **testmp3s();
void *mp3merge(mp3info_t **, int);
void catmp3(mp3info_t **, int , char *);

/* mutex setup ----------------------------------------------------- */
pthread_mutex_t lock_createthreads = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_makeindex = PTHREAD_MUTEX_INITIALIZER;

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
        exit(EXIT_FAILURE);
    }

    /* spin off top thread ----------------------------------------- */
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

    fprintf(stdout, "main: initialized top thread: %#lx, dir: %s\n", topid, top_name); // TODO: remove debug

    /* wait for top thread ----------------------------------------- */
    pthread_join(topid, NULL);
    
    /* merge mp3s -------------------------------------------------- */
    // TODO: take user input for output filename

    // void *outputdata = testmp3s(); // TODO: remove debug
    // catmp3(outputdata, 8, DEFAULTOUTFN); // TODO: remove debug
    // void *mergedmp3s = mp3merge(outputdata, 8); // TODO: remove

    // /* create ouput file */
    // const char *outputfilename = DEFAULTOUTFN; // TODO: remove debug
    // FILE *output_mp3 = fopen(outputfilename, "wb");

    // /* write merged contents to the file */
    // if (fwrite(mergedmp3s, sizeof(*mergedmp3s), 1, output_mp3) != sizeof(*mergedmp3s))
    // {
    //     perror("main: failed to write mp3 output file");
    //     exit(EXIT_FAILURE);
    // }

    // fprintf(stdout, "main: created test output\n"); // TODO: remove debug

    // if (fclose(output_mp3) != 0)
    // {
    //     fprintf(stderr, "parser::main: file not successfully closed: (\"%s\")\n", outputfilename);
    //     exit(EXIT_FAILURE);
    // }

	return 0;
} // main #############################################################

/* thread func to search a dir for mp3 files */
void *searchdir(void *arg){
    dir_t *current = (dir_t *)arg;

    if (current->dir == NULL)
    {
        fprintf(stderr, "searchdir: dir is NULL; tid: %#lx; pid: %u\n", pthread_self(), getppid());
        exit(EXIT_FAILURE);
    }

    int localthreadcount = 0;
    pthread_t *localtids = NULL;

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

            if (next_dir->dir == NULL)
            {
                fprintf(stderr, "searchdir: path: \"%s\"; tid: %#lx >>> ", next_dir->path, pthread_self());
                perror("opendir failed");
                exit(EXIT_FAILURE);
            }
            
            pthread_mutex_lock(&lock_createthreads); // ***************
            fprintf(stdout, "searchdir: tid: %#lx; subdir \"%s\"\n", pthread_self(), next_dir->path); // TODO: remove debug
            
            localtids = realloc(localtids, sizeof(pthread_t) * ++localthreadcount);

            int err = pthread_create(&localtids[localthreadcount - 1], NULL, searchdir, next_dir);
            
            if (err != 0) 
            {
                perror("searchdir: could not create thread");
                exit(EXIT_FAILURE);
            }
            pthread_mutex_unlock(&lock_createthreads); // *************
		} 
        else if (is_mp3(dir_entry->d_name)) // check if file ext is mp3
		{
            pthread_mutex_lock(&lock_makeindex); // *******************
			makeindex(dir_entry, current->path);
            pthread_mutex_unlock(&lock_makeindex); // *****************
		} // else file was other type not used here, skip it
	}

    for (size_t i = 0; i < localthreadcount; i++)
    {
        pthread_join(localtids[i], NULL);
    }

    return NULL;
}

/* make an index entry in global mp3_entries */
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
    FILE *contents = fopen(mp3_index[file_int]->path, "rb");

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

    fprintf(stdout, "makeindex: tid: %#lx sucessfully created index: id: %d\n", pthread_self(), mp3_index[file_int]->filename);
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
        off_t contents_size = getfilesize(output[i]->path);

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
    off_t totalsize = 0; // sum of the sizes of the individual segments
    for (size_t i = 0; i < mp3indexlen; i++) // loop to sum each segment
    {
        totalsize += getfilesize(mp3index[i]->path);
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

/* pipeline to run cat

    taken from lecture, CS3240, Spring 2021, Macreery
*/
static void pipeline(char *** cmd)
{
    int fd[2];
    pid_t pid;
    int fdd = 0;

    while (*cmd != NULL){
        pipe(fd);
        if ((pid = fork()) == ERROR)
        {
            perror("pipeline: fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            dup2(fdd, STDIN_FILENO);
            if (*(cmd + 1) != NULL)
            {
                dup2(fd[1], STDOUT_FILENO);
            }
            close(fd[0]);

            execvp((*cmd)[0], *cmd);
            exit(EXIT_FAILURE);
        }
        else
        {
            wait(NULL);
            close(fd[1]);
            fdd = fd[0];
            cmd++;
        }
    }
}

/* concatenate mp3 segments using cat */
void catmp3(mp3info_t **mp3index, int mp3indexlen, char *outputfilename)
{
    int len = mp3indexlen + 2; // index length plus 4 args

    char *cat[len];

    cat[0] = "cat";
    cat[len - 1] = NULL;

    for (size_t i = 0; i < mp3indexlen; i++)
    {
        cat[i+1] = strdup(mp3index[i]->path);
    }

    char *redirect[] = {">", outputfilename, NULL};

    char **cmd[] = {cat, redirect, NULL};

    pipeline(cmd);
    
    // TODO: free argv
}

/* free functions -------------------------------------------------- */
// TODO: make free functions
