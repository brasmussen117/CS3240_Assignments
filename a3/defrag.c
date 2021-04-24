#include "defrag_header.h"

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

	/* top loop ---------------------------------------------------- */

	struct dirent *dir_entry = NULL;
	
	while (true)
	{
		dir_entry = readdir(top_dir);

		if (dir_entry == NULL) break; // check if dir_entry is NULL, end loop
		
		if (dir_entry->d_type == DT_DIR) // check if dir_entry entry is subdir
		{
			if ((strcmp(dir_entry->d_name, ".")==0) || // check if self or parent pointers
				(strcmp(dir_entry->d_name, "..")==0))
			{
				continue; // skip self or parent pointers
			}
			
			/* spin off thread with subdir */
			printf("dir_entry.d_name: %s", dir_entry->d_name); //TODO: remove debug
		}

		if (is_mp3(dir_entry->d_name)) // check if file ext is mp3
		{ 
			printf("mp3 file found: %s", dir_entry->d_name); //TODO: remove debug
			// handle the file

		} // else file was other type not used here, skip it
	}
		

	return 0;
}
