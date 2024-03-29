#include "defrag_header.h"

/* mutex setup ----------------------------------------------------- */
pthread_mutex_t lock_createthreads = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_makeindex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char const *argv[]) // #############################
{
    /* check for necessary arguments ------------------------------- */
    if (argc < 3) // check for minimum args
    {
        fprintf(stderr, "Usage: <dir> <output_filename>\n");
        exit(EXIT_FAILURE);
    }
    if (argc > 3) // check that no extra args given
    {
        errno = E2BIG;
        fprintf(stderr, "./defrag: Arg list too long\n");
        exit(EXIT_FAILURE);
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
    
    free(next_dir);

    if (err != 0) 
    {
        perror("ERROR: main: could not create thread");
        exit(EXIT_FAILURE);
    }

    /* wait for top thread ----------------------------------------- */
    pthread_join(topid, NULL);
    
    /* merge mp3s -------------------------------------------------- */
    catmp3(mp3_index, mp3_index_length, strdup(argv[2]));

	return 0;
} // main #############################################################

/* thread func to search a dir for mp3 files */
void *searchdir(void *arg){
    dir_t *current = (dir_t *)arg;

    if ((current->dir == NULL) || (current->path == NULL)) // check if input is good
    {
        fprintf(stderr, "searchdir: current dir and/or path is NULL; tid: %#lx\n", pthread_self());
        (current->dir) ? fprintf(stderr, "dir is good\n") : fprintf(stderr, "dir is NULL\n");
        (current->path) ? fprintf(stderr, "path: %s\n", current->path) : fprintf(stderr, "path is NULL\n");
        exit(EXIT_FAILURE);
    }

    int localtn = 0; // number of local threads
    pthread_t *localtids = NULL;

    /* prep -------------------------------------------------------- */
    struct dirent *dir_entry = NULL;

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
            pthread_mutex_lock(&lock_createthreads); // ***************
            
            dir_t *next_dir = malloc(sizeof(dir_t));

            next_dir->path = catpath(dir_entry->d_name, current->path);

            next_dir->dir = opendir(next_dir->path);

            if (next_dir->dir == NULL)
            {
                fprintf(stderr, "ERROR: searchdir: path: \"%s\" - tid: %#lx\n\t>>> ", next_dir->path, pthread_self());
                perror("opendir failed");
                exit(EXIT_FAILURE);
            }
            
            localtids = realloc(localtids, sizeof(pthread_t) * ++localtn);

            int err = pthread_create(&localtids[localtn - 1], NULL, searchdir, next_dir);
            
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
	} // end while loop

    for (size_t i = 0; i < localtn; i++) // loop to join threads
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

    mp3_index_length++; // inc mp3_index_length
    
    fprintf(stdout, "makeindex: tid: %#lx sucessfully created index: id: %d\n", pthread_self(), mp3_index[file_int]->filename);
}

/* pipeline to run cat

    taken from lecture, CS3240, Spring 2021, Macreery
    and StackOverflow
        https://stackoverflow.com/a/37307304/13305483
        author: https://stackoverflow.com/users/6086833/t-johnson
*/
static void catpipe(char *** cmd, char *filename)
{
    int fd[2];
    pid_t pid;

    if(pipe(fd) == ERROR)
    {
        perror("catpipe: pipe failed");
        exit(EXIT_FAILURE);
    }
    else if ((pid = fork()) == ERROR)
    {
        perror("pipeline: fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) // child process
    {
        if (dup2(fd[0], STDIN_FILENO) == ERROR)
        {
            perror("catpipe: dup2 failed on stdin");
            exit(EXIT_FAILURE);
        }

        close(fd[0]);

        FILE *output = fopen(filename, "w");

        if (output == NULL)
        {
            perror("catpipe: fopen failed on output");
            exit(EXIT_FAILURE);
        }
        
        if (dup2(fileno(output), STDOUT_FILENO) == ERROR)
        {
            perror("catpipe: dup2 failed on stdout > output");
            exit(EXIT_FAILURE);
        }
        
        fclose(output);

        execvp((*cmd)[0], *cmd);
        
        /* should not pass here ------------------------------------ */
        fprintf(stderr, "catpipe: execvp failed");
        perror("");
        exit(EXIT_FAILURE);
    }
    else // parent process
    {
        wait(NULL);
        close(fd[1]);
    }
}

/* concatenate mp3 segments using pipe>exec>cat */
void catmp3(mp3info_t **mp3index, int mp3indexlen, char *outputfilename)
{
    int len = mp3indexlen + 2; // index length extra args

    char *cat[len];

    cat[0] = "cat";
    cat[len - 1] = NULL;

    for (size_t i = 0; i < mp3indexlen; i++)
    {
        cat[i+1] = strdup(mp3index[i]->path);
    }

    // char *redirect[] = {">", outputfilename, NULL};

    char **cmd[] = {cat, NULL};

    catpipe(cmd, outputfilename);
    
    for (size_t i = 1; i < (mp3indexlen+1); i++)
    {
        free(cat[i]);
    }
}
