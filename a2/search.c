#include "cardfuncs.h"

/* function prototypes */
void cleanInput(char[]);
int search(char *, INDEXARR *);
INDEXARR *readIndexBin(FILE *);

int main(int argc, char const *argv[])
{
	/* check for necessary arguments ------------------------------- */
	if (argc > 1) // check that no extra args given
	{
		errno = E2BIG;
		perror("./search");
		return 1;
	}

	/* open index -------------------------------------------------- */
	FILE *indexbin = fopen(INDEXBINFN, "rb");

	if (indexbin == NULL) // check that fopen was sucessful
	{
		if (errno == ENOENT)
		{
			fprintf(stderr, "./search: cannot open(\"%s\"): No such file or directory\n", INDEXBINFN);
		}
		else
		{
			perror("./search::main: cannot open file");
		}
		return 1;
	}

	/* build index from bin ---------------------------------------- */
	INDEXARR *indices = readIndexBin(indexbin); // arr struct of indexes

	/* close index file -------------------------------------------- */
	if (fclose(indexbin) != 0)
	{
		perror("./search::main: failed to close index.bin");
	}

	/* I/O loop ---------------------------------------------------- */
	char userinput[MAXLINE]; // take user input up tot MAXLINE
	int result;			 // get result of search

	if (isatty(STDIN_FILENO) != TERMINAL) // check if input is redirected
	{
		fprintf(stdout, ">> "); // display prompt
		fgets(userinput, MAXLINE, stdin);
		while ((userinput != NULL) && strcmp(userinput, "")!=0)
		{
			cleanInput(userinput); // strip non-chars

			fprintf(stdout, "%s\n", userinput); // echo input

			if ((strlen(userinput) == 1) && (strcmp(userinput, "q")==0 || strcmp(userinput, "Q")==0))
			{
				break;
			}
			
			result = search(userinput, indices);

			if (result == -1)
			{
				fprintf(stdout, "./search: '%s' not found!\n", userinput);
			}

			fprintf(stdout, ">> "); // display prompt
			fgets(userinput, MAXLINE, stdin);
		}
	} else // input is terminal
	{
		while (1)
		{
			fprintf(stdout, ">> "); // display prompt
			if (fgets(userinput, MAXLINE, stdin) == NULL) // try to get user input
			{
				perror("./search::main: fgets failed to get user input");
				return 2;
			}
			
			cleanInput(userinput);

			if ((strlen(userinput) == 1) && ((*userinput == 'q') || (*userinput == 'Q'))) // check for escape character
			{
				break;
			}

			result = search(userinput, indices);

			if (result == -1)
			{
				fprintf(stdout, "./search: '%s' not found!\n", userinput);
			}
		}
	}

	/* free memory ------------------------------------------------- */
	freeindices_name(indices);

	return 0;
}

/* clean the end of the redirected input */
void cleanInput(char userinput[MAXLINE])
{
	char *cr = strstr(userinput, "\r");
	if (cr != NULL)
	{
		cr[0] = 0;
	}

	char *nl = strstr(userinput, "\n");
	if (nl != NULL)
	{
		nl[0] = 0;
	}
}

/* read index.bin and return built INDEXARR* */
INDEXARR *readIndexBin(FILE *indexbin)
{
	/* read size of indices ---------------------------------------- */
	// uint32_t *indexsize = malloc(sizeof(uint32_t));			  // number of indices in bin
	// if (fread(indexsize, sizeof(uint32_t), 1, indexbin) != 1) // try to read indexsize, handle if not able
	// {
	// 	perror("./search::readIndexBin: cannot read file");
	// 	exit(EXIT_FAILURE);
	// } // TODO: remove

	/* initialize indices ------------------------------------------ */
	INDEXARR *indices = malloc(sizeof(INDEXARR));
	// indices->arr = malloc(sizeof(INDEX *) * *indexsize); // statically allocated
	indices->arr = malloc(sizeof(INDEX *)); // dynamically allocated
	// indices->size = indexsize;

	uint32_t *len = malloc(sizeof(uint32_t));

	// for (uint32_t i = 0; i < *indices->size; i++) // loop to read each index entry
	size_t i = 0;
	while (1)
	{
		if (fread(len, sizeof(uint32_t), 1, indexbin) != 1) // try to read strlen
		{
			// fprintf(stderr, "./search::readIndexBin: cannot read size: loop-%d\n", i);
			// exit(EXIT_FAILURE); // TODO: REMOVE

			break;
		}

		indices->arr[i] = malloc(sizeof(INDEX)); // malloc new index entry
		indices->arr[i]->name = malloc(sizeof(char) * ((size_t)*len + 1)); // malloc new name with new len

		if (fread(indices->arr[i]->name, sizeof(char), *len, indexbin) != *len) // try to read name
		{
			fprintf(stderr, "./search::readIndexBin: cannot read name: loop-%ld\n", i);
			exit(EXIT_FAILURE);
		}

		indices->arr[i]->name[*len] = 0; // null terminate

		indices->arr[i]->offset = malloc(sizeof(long)); // malloc new offset

		if (fread(indices->arr[i]->offset, sizeof(long), 1, indexbin) != 1) // try to read offset
		{
			fprintf(stderr, "./search::readIndexBin: cannot read offset: loop-%ld\n", i);
			exit(EXIT_FAILURE);
		}

		i++;
		indices->arr = realloc(indices->arr, sizeof(INDEX*) * (i + 1));
	}

	indices->size = convtoptr_uint32_t((uint32_t)i);

	free(len);

	return indices;
}

/* read cards.bin
	return new CARD*
 */
CARD *readCardBin(char *cardbin_filename, INDEX **index)
{
	/* open bin file ----------------------------------------------- */
	FILE *cardbin = fopen(cardbin_filename, "rb"); // bin file pointer
	if (cardbin == NULL) // check if fopen successful
	{
		perror("./search::readCardBin: failed to open bin file");
		exit(EXIT_FAILURE);
	}

	fseek(cardbin, *index[0]->offset, SEEK_SET); // point the file offset to value from index

	uint32_t *len = malloc(sizeof(uint32_t)); // reusable var to hold length of each char* field

	/* vars for reading and building newcard ----------------------- */
	uint32_t *id = malloc(sizeof(uint32_t)); // id field, mem allocated
	char *cost; // cost field
	uint32_t *converted_cost = malloc(sizeof(uint32_t)); //converted_cost field, mem allocated
	char *type; // type field
	char *text; // text field
	char *stats; // stats field
	uint32_t *rarity_uint32_t = malloc(sizeof(uint32_t)); // temp rarity, mem allocated
	RARITY rarity;

	// #region read fields -----------------------------------------

	/* id field */
	if (fread(id, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read id field");
	}

	/* cost len */
	if (fread(len, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read cost-len");
	}

	/* cost field */
	cost = malloc(sizeof(char) * *len + 1);
	if (fread(cost, sizeof(char), (size_t)*len, cardbin) != *len)
	{
		perror("./search::readCardBin: failed to read cost field");
	}
	cost[*len] = 0; // null terminate

	/* converted_cost field */
	if (fread(converted_cost, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read converted_cost field");
	}

	/* type len */
	if (fread(len, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read type-len");
	}

	/* type field */
	type = malloc(sizeof(char) * *len + 1);
	if (fread(type, sizeof(char), (size_t)*len, cardbin) != *len)
	{
		perror("./search::readCardBin: failed to read type field");
	}
	type[*len] = 0; // null terminate

	/* text len */
	if (fread(len, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read text-len");
	}

	/* text field */
	text = malloc(sizeof(char) * *len + 1);
	if (fread(text, sizeof(char), (size_t)*len, cardbin) != *len)
	{
		perror("./search::readCardBin: failed to read text field");
	}
	text[*len] = 0; // null terminate

	/* stats len */
	if (fread(len, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read stats-len");
	}

	stats = malloc(sizeof(char) * *len + 1);
	if (fread(stats, sizeof(char), (size_t)*len, cardbin) != *len)
	{
		perror("./search::readCardBin: failed to read stats field");
	}
	stats[*len] = 0; // null terminate

	/* rarirty_uint32_t field */
	if (fread(rarity_uint32_t, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read rarity field");
	}
	rarity = (RARITY)rarity_uint32_t[0]; // convert uint32_t to rarity
	
	// #endregion

	/* close file -------------------------------------------------- */
	if (fclose(cardbin) == EOF)
	{
		perror("./search::readCardBin: failed to close bin file");
		exit(EXIT_FAILURE);
	}

	/* build newcard ----------------------------------------------- */
	char *card_name = strdup(index[0]->name);
	CARD *newcard = cardBuilder((unsigned int)*id, card_name, cost, (unsigned int)*converted_cost, type, text, stats, rarity);

	/* free memory ------------------------------------------------- */
	free(len);
	free(id);
	free(converted_cost);
	free(rarity_uint32_t);

	return newcard;
}

/* search the index for userinput
	return -1 if no match
		0 if good match
 */
int search(char *userinput, INDEXARR *indices)
{
	int ret;

	INDEX **input_index = malloc(sizeof(INDEX*));
	input_index[0] = malloc(sizeof(INDEX));
	input_index[0]->name = strndup(userinput, strlen(userinput) + 1);

	INDEX **found_index = NULL;

	void *result = bsearch(input_index, indices->arr, (size_t)*indices->size, sizeof(INDEX *), comparIndexNames);

	if (result == NULL)
	{
		ret = -1; // failed to find match

		free(found_index);
	} else
	{
		ret = 0; // good match found

		found_index = result;

		CARD *found_card = readCardBin(CARDBINFN, found_index); // read found_card from found_index

		if (found_card == NULL) // check that found_card read properly
		{
			exit(EXIT_FAILURE);
		}

		printCard(found_card); // print the card

		freeCard(found_card); // free the card

		// *found_index = NULL;

		// if (realloc(found_index, 0) != NULL)
		// {
		// 	free(found_index);
		// }
	}

	free(input_index[0]->name);
	free(input_index[0]);
	free(input_index);

	return ret;
}

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
