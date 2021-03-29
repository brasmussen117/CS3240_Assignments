#include "cardfuncs.h"

#define fn "s_indexbin"

int main(int argc, char const *argv[])
{
	/* check for necessary arguments ------------------------------- */
    if (argc > 1) // check that no extra args given
    {
        errno = E2BIG;
        fprintf(stderr, "./search: Arg list too long\n");
        return 1;
    }

	/* open index -------------------------------------------------- */
	FILE *indexbin = fopen(fn, "rb");

	if (indexbin == NULL) // check that fopen was sucessful
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "./search: cannot open(\"%s\"): No such file or directory\n", argv[1]);
        }
        else
        {
			perror("./search: cannot open file");
        }
        return 1;
    }

	/* build index from bin ---------------------------------------- */
	uint32_t *indexsize = malloc(sizeof(uint32_t)); // number of indices in bin
	if (fread(indexsize, sizeof(uint32_t), 1, indexbin) != 1) // try to read indexsize, handle if not able
	{
		perror("./search: cannot read file");
		return 2;
	}

	INDEX **indices = malloc(sizeof(INDEX*) * *indexsize);
	// CARDARR *cards = malloc(sizeof(CARDARR));
	// cards->arr = malloc(sizeof(CARD*) * *indexsize);
	// cards->size = *indexsize;
	uint32_t *len = malloc

	for (uint32_t i = 0; i < *indexsize; i++)
	{
		
		indices[i]->name
	}
	
	

	/* UI loop ----------------------------------------------------- */
	char *userinput[100]; // take user input
	while ((userinput != "q") && (userinput != "Q"))
	{
		fprintf(stdout, ">>");
		scanf("%s", userinput);


	}
	

	return 0;
}

// int getuserinput(){

// }

/* 
# Search
1. Reading the entire index file (`index.bin`). every entry should be placed into a struct and into an array.
2. It should already be sorted. Read user input and call `bsearch()` (binary search) for that card name.
3. If a match is found, read the byte offset that is associated with that card name. Read from the other file (`cards.bin`) to obtain the rest of the card data and assemble the `CARD` record. Print it to the screen.
4. Running the program and typing input in to the terminal is different than when input is redirected to the program. During input redirection input is not echoed out to the terminal. In order to present to the tester a view of your program closer to when it is executed normally, if the input is not from a terminal, you must echo the inputted line to `stdout`! This can be accomplished by calling the function `isatty(0)`, please read the manpage for this, but basically it will return a 1 if input is coming from a terminal, otherwise it will return 0 (and also set `errno`, but we don't need that information).

$ ./search

>> Eternal Isolation
Eternal Isolation                   {1}{W}
Sorcery                           uncommon
------------------------------------------
Put target creature with power 4 or greater on the bottom of its owner's library.
------------------------------------------
                                          

>> Does not exist
./search: 'Does not exist' not found!
>> Corpse Knight
Corpse Knight                       {W}{B}
Creature - Zombie Knight          uncommon
------------------------------------------
Whenever another creature enters the battlefield under your control, each opponent loses 1 life.
------------------------------------------
                                       2/2

>> q

# Example 4
$ make && ./search

There is nothing printed to stdout, shown below is what is printed to stderr.

./search: cannot open("index.bin"): No such file or directory
$ touch index.bin && ./search

There is nothing printed to stdout, shown below is what is printed to stderr.

./search: cannot open("cards.bin"): No such file or directory
 */
