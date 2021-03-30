#include "cardfuncs.h"

/* function prototypes */
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
	FILE *indexbin = fopen(indexbinfn, "rb");

	if (indexbin == NULL) // check that fopen was sucessful
	{
		if (errno == ENOENT)
		{
			fprintf(stderr, "./search: cannot open(\"%s\"): No such file or directory\n", argv[1]);
		}
		else
		{
			perror("./search::main: cannot open file");
		}
		return 1;
	}

	/* build index from bin ---------------------------------------- */
	INDEXARR *indices = readIndexBin(indexbin);

	/* close index file -------------------------------------------- */
	if (fclose(indexbin) != 0)
	{
		perror("./search::main: failed to close index.bin");
	}

	/* UI loop ----------------------------------------------------- */
	char userinput[100]; // take user input
	int result;			 // get result of search

	while (!((strlen(userinput) == 1) && ((*userinput == 'q') || (*userinput == 'Q'))))
	{
		fprintf(stdout, ">> "); // display prompt
		if (fscanf(stdin,"%s", userinput) <= 0)
		{
			perror("./search::main: scanf failed to get user input");
		}

		if (isatty(STDIN_FILENO) == REDIRECT)
		{
			fprintf(stdout, "%s\n", userinput);
		}

		result = search(userinput, indices);

		if (result == -1)
		{
			fprintf(stdout, "./search: '%s' not found!\n", userinput);
		}
	}

	/* free memory ------------------------------------------------- */
	freeindices(indices);

	return 0;
}

/* read index.bin and return built INDEXARR* */
INDEXARR *readIndexBin(FILE *indexbin)
{
	/* read size of indices ---------------------------------------- */
	uint32_t *indexsize = malloc(sizeof(uint32_t));			  // number of indices in bin
	if (fread(indexsize, sizeof(uint32_t), 1, indexbin) != 1) // try to read indexsize, handle if not able
	{
		perror("./search::readIndexBin: cannot read file");
		exit(EXIT_FAILURE);
	}

	/* initialize indices ------------------------------------------ */
	INDEXARR *indices = malloc(sizeof(INDEXARR));
	indices->arr = malloc(sizeof(INDEX *) * *indexsize);
	indices->size = indexsize;

	uint32_t *len = malloc(sizeof(uint32_t));

	for (uint32_t i = 0; i < *indices->size; i++) // loop to read each index entry
	{
		indices->arr[i] = malloc(sizeof(INDEX)); // malloc new index entry

		if (fread(len, sizeof(uint32_t), 1, indexbin) != 1) // try to read strlen
		{
			fprintf(stderr, "./search::readIndexBin: cannot read size: loop-%d\n", i);
			exit(EXIT_FAILURE);
		}

		indices->arr[i]->name = malloc(sizeof(char) * (size_t)*len); // malloc new name with new len

		if (fread(indices->arr[i]->name, sizeof(char), *len, indexbin) != *len) // try to read name
		{
			fprintf(stderr, "./search::readIndexBin: cannot read name: loop-%d\n", i);
			exit(EXIT_FAILURE);
		}

		indices->arr[i]->offset = malloc(sizeof(long)); // malloc new offset

		if (fread(indices->arr[i]->offset, sizeof(long), 1, indexbin) != 1) // try to read offset
		{
			fprintf(stderr, "./search::readIndexBin: cannot read offset: loop-%d\n", i);
			exit(EXIT_FAILURE);
		}
	}

	free(len);

	return indices;
}

/* read cards.bin
	return new CARD*
 */
CARD *readCardBin(char *cardbin_filename, INDEX *index)
{
	FILE *cardbin = fopen(cardbin_filename, "rb");
	if (cardbin == NULL)
	{
		perror("./search::readCardBin: failed to open bin file");
		exit(EXIT_FAILURE);
	}

	fseek(cardbin, *index->offset, SEEK_SET); // point the file offset to value from index

	uint32_t *len = malloc(sizeof(uint32_t)); // hold length of each char* for reading from file

	/* vars for reading and building newcard */
	uint32_t *id = malloc(sizeof(uint32_t)); // id field, mem allocated
	char *cost; // cost field
	uint32_t *converted_cost = malloc(sizeof(uint32_t)); //converted_cost field, mem allocated
	char *type; // type field
	char *text; // text field
	char *stats; // stats field
	uint32_t *rarity_uint32_t = malloc(sizeof(uint32_t)); // temp rarity, mem allocated
	RARITY rarity;

	// #region read fields

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
	cost = malloc(sizeof(char) * *len);
	if (fread(cost, sizeof(char), (size_t)*len, cardbin) != *len)
	{
		perror("./search::readCardBin: failed to read cost field");
	}

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
	type = malloc(sizeof(char) * *len);
	if (fread(type, sizeof(char), (size_t)*len, cardbin) != *len)
	{
		perror("./search::readCardBin: failed to read type field");
	}

	/* text len */
	if (fread(len, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read text-len");
	}

	/* text field */
	text = malloc(sizeof(char) * *len);
	if (fread(text, sizeof(char), (size_t)*len, cardbin) != *len)
	{
		perror("./search::readCardBin: failed to read text field");
	}

	/* stats len */
	if (fread(len, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read stats-len");
	}

	/* stats field */
	stats = malloc(sizeof(char) * *len);
	if (fread(stats, sizeof(char), (size_t)*len, cardbin) != *len)
	{
		perror("./search::readCardBin: failed to read stats field");
	}

	/* rarirty_uint32_t field */
	if (fread(rarity_uint32_t, sizeof(uint32_t), 1, cardbin) != 1)
	{
		perror("./search::readCardBin: failed to read rarity field");
	}
	rarity = (RARITY)rarity_uint32_t; // convert uint32_t to rarity
	
	// #endregion

	/* build newcard */
	CARD *newcard = cardBuilder((unsigned int)*id, strdup(index->name), cost, (unsigned int)*converted_cost, type, text, stats, rarity);

	/* free memory */
	free(id);
	free(converted_cost);
	free(rarity_uint32_t);

	return newcard;
}

/* custom comparator for bsearch 
	returns strmp of index entry names 
*/
int comparIndexNames(const void *indexA, const void *indexB)
{
	const INDEX *A = *(INDEXARR.arr)indexA;
	const INDEX *B = *(INDEX **)indexB;
	return strcmp(A->name, B->name);
}

/* search the index for userinput
	return -1 if no match
		0 if good match
 */
int search(char *userinput, INDEXARR *indices)
{
	INDEX *found_index = NULL;
	CARD *found_card = NULL;

	found_index = bsearch(userinput, indices->arr, (size_t)indices->size, sizeof(INDEX *), comparIndexNames);

	if (found_index == NULL)
	{
		return -1; // failed to find match
	}

	found_card = readCardBin(cardbinfn, found_index);

	printCard(found_card);

	freeCard(found_card);

	return 0;
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
