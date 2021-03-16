#include "card.h"

// #region function prototypes
CARDARR *parse_file_csv(FILE *);
int checkDuplicateCard(CARD *, CARDARR *);
int comparCardName(const void *, const void *);
CARD *cardBuilder(unsigned, char *, char *, unsigned, char *, char *, char *, RARITY);
RARITY strToRARITY(char *);
void closeCardGap(CARDARR *, int);
void convNewlineChar(char *);
void printCard(CARD *);
void printCards(CARDARR *);
void freeCard(CARD *);
void freeCards(CARDARR *);
CARD *parse_line_csv(char *);
char *cleanstr(char *);
void stripFieldQuotes(char *);
void stripDoubledQuotes(char *);
void closeStrGap(char *, char *);
// #endregion

int main(int argc, char const *argv[])
{
    /* check for necessary arguments --------------------------- */
    if (argc < 2) // check that filename was given
    {
        fprintf(stderr, "Usage: ./parser <file_name>\n");
        return 1;
    }
    if (argc > 2) // check that no extra args given
    {
        errno = E2BIG;
        fprintf(stderr, "./parser: Arg list too long");
        return 1;
    }

    /* open file ----------------------------------------------- */
    FILE *input_file = fopen(argv[1], "r"); // open the file given as argument in command line

    if (input_file == NULL) // check that fopen was sucessful
    {
        if (errno == ENOENT)
        {
            fprintf(stderr, "./parser: cannot open(\"%s\"): No such file or directory\n", argv[1]);
        }
        else
        {
            fprintf(stderr, "./parser: cannot open(\"%s\"): error: %d\n", argv[1], errno);
        }
        return 1;
    }

    CARDARR *cards = parse_file_csv(input_file);

    /* close file -------------------------------------------------- */
    if (fclose(input_file) != 0) // check file close success
    {
        fprintf(stderr, "Error: file not successfully closed\n");
        return 2;
    }

    /* sort cards -------------------------------------------------- */
    qsort(cards->arr, cards->count, sizeof(CARD *), comparCardName);

    /* print cards ------------------------------------------------- */
    printCards(cards);

    /* free memory ------------------------------------------------- */
    freeCards(cards);

    return 0;
}

// #region functions

/* loop and read file, puts cards into **cards */
CARDARR *parse_file_csv(FILE *input_file)
{
    CARDARR *cards = malloc(sizeof(CARDARR));
    cards->arr = NULL;
    cards->count = 0;

    /* setup and get first line -------------------------------- */
    char *buf = NULL; // buffer
    size_t n = 0;     // size of line

    ssize_t linesize = getline(&buf, &n, input_file); // read the first line, header unused

    if (linesize < 0) // check that the file is not empty
    {
        fprintf(stderr, "Error: file emtpy or getline error\n");
        fclose(input_file);
        return cards;
    }

    CARD *newCard = NULL; // pointer for a new card

    ssize_t dup = 0; // duplicate card flag

    // size_t count = 0; // main counter

    while (linesize > 0) // read/parse loop
    {
        linesize = getline(&buf, &n, input_file); // read the next line

        if (linesize <= 0) // check if there was a new line
        {
            break; // break if line empty or error
        }

        newCard = parse_line_csv(buf);

        /* check for and handle duplicates ------------------------- */
        if (cards->count > 0) // check if first loop
        {
            dup = checkDuplicateCard(newCard, cards); // check for duplicate
            if (dup != -1) // if dup returns a match then handle it
            {
                if (dup == -2) // check if new entry is not superseding, skip this entry if not
                {
                    freeCard(newCard);
                    continue; // skip to next iteration
                }
                else // new entry IS superseding, dup has index of obs card
                {
                    closeCardGap(cards, dup); // close the gap to remove obs card
                }
            } // else no duplicate found,  proceed
        } // else first loop, proceed

        cards->arr = realloc(cards->arr, sizeof(CARD *) * ++cards->count); // realloc cards, pre-increment count

        cards->arr[cards->count - 1] = newCard; // put newCard into cards
    } // end read loop

    free(buf);

    return cards;
}

/* take buf line input and parse 
    returns pointer to new card
 */
CARD *parse_line_csv(char *buf)
{
    unsigned int id = 0;             // var for bulding new card
    char *name = NULL;               // var for bulding new card
    char *cost = NULL;               // var for bulding new card
    unsigned int converted_cost = 0; // var for bulding new card
    char *type = NULL;               // var for bulding new card
    char *text = NULL;               // var for bulding new card
    char *stats = NULL;              // var for bulding new card
    RARITY rarity;                   // var for bulding new card

    char *line = strdup(buf); // dup the buf for strsep
    char *free_line = line;   // pointer for cleaning memory in loop
    char *rarityStr = NULL;   // take rarity as string for conversion
    char *comma = NULL;       // ptr for examining comma postions

    ssize_t offset = 0; // ptr for str separating

    /* begin parsing input ------------------------------------------- */

    id = atoi(strsep(&line, ",")); // sep id field, convert to int

    comma = strstr(line, ","); // find position of next comma
    if (
        (((char *)(comma - 1))[0] == '\"') && // check if there are double quotes on either side of comma
        (((char *)(comma + 1))[0] == '\"'))
    {                                      // comma is between double quotes
        name = strdup(strsep(&line, ",")); // sep/dup name field
    }
    else // comma was not betweenn double quotes, presume comma inside name field
    {
        line++;                             // advance past the beginning dq
        name = strdup(strsep(&line, "\"")); // sep/dup name field
        line++;                             // advance past the comma
    }

    cost = strdup(strsep(&line, ",")); // sep/dup cost field

    converted_cost = atoi(strsep(&line, ",")); // sep converted_cost field, convert to int

    type = strdup(strsep(&line, ",")); // sep/dup type field

    /* backtrack from the end to get text field ---------------------- */

    offset = (strlen(line)); // set offset to the end of line, pre-increment past first quote

    while (line[offset] != ',')
    { // march back to the comma before rarity
        offset--;
    }

    offset--; // set before last comma

    while (line[offset] != ',')
    { // march offset to the comma before stats
        offset--;
    }

    text = strndup(line, offset); // dup the text field

    line = line + offset + 1; // point to first char of stat field

    /* strsep the remainder as before -------------------------------- */

    stats = strdup(strsep(&line, ",")); // sep/dup stats field

    if (strstr(line, "\r"))
    {
        rarityStr = strdup(strsep(&line, "\r")); // sep/dup rarityStr
    }
    else
    {
        rarityStr = strdup(strsep(&line, "\n")); // sep/dup rarityStr
    }

    /* clean up, build the new card, and return ---------------------- */
    rarity = strToRARITY(cleanstr(rarityStr)); // clean str, convert rarityStr to RARITY
    free(rarityStr);                           // free rarityStr

    free(free_line); // free memory

    return cardBuilder(
        id,
        cleanstr(name),
        cleanstr(cost),
        converted_cost,
        cleanstr(type),
        cleanstr(text),
        cleanstr(stats),
        rarity);
}

/* frees a CARD* and char* fields
    checks if certain fields are pointing to zptr:
        cost, text, stats
 */
void freeCard(CARD *cardtofree)
{
    /* free the cards's char* fields */
    free(cardtofree->name);
    if (cardtofree->cost != zptr)
    {
        free(cardtofree->cost);
    }
    free(cardtofree->type);
    if (cardtofree->text != zptr)
    {
        free(cardtofree->text);
    }
    if (cardtofree->stats != zptr)
    {
        free(cardtofree->stats);
    }

    free(cardtofree); // free the card
}

/* frees full CARD** array */
void freeCards(CARDARR *cards)
{
    /* loop to free each of the cards */
    for (size_t i = 0; i < cards->count; i++)
    {
        freeCard(cards->arr[i]);
    }
    free(cards->arr); // free the full array
    free(cards);
}

/* take an allocated string and clean as needed
    checks if null, return zptr
    calls convNewlineChar, stripFieldQuotes, stripDoubledQuotes as needed
    returns realloc'd ptr
 */
char *cleanstr(char *s)
{
    if ((s != zptr) && (s[0] != '\0'))
    {
        stripFieldQuotes(s); // strip the quotes from beginning/end

        stripDoubledQuotes(s); // strip doubled double quotes ("") to be a single double quote

        convNewlineChar(s); // strip characters '\' & 'n' and places special \n character

        return s;
    }
    else // field was empty
    {
        free(s); // free the allocated input

        return zptr; // return pointer to zero
    }
}

/* take char*, find '\n' as characters, convert to special character \n */
void convNewlineChar(char *s)
{

    char *ptr = strstr(s, "\\");

    if (ptr != NULL)
    {
        while (ptr != NULL)
        {
            if (((char *)(ptr + 1))[0] == 'n')
            {
                closeStrGap(ptr, (ptr + 1));
                ptr[0] = '\n';
            }

            ptr++;

            ptr = strstr(ptr, "\\");
        }
    }
}

/* takes a string and strips off doubled double quotes */
void stripDoubledQuotes(char *s)
{

    char *ptr = strstr(s, "\"");

    if (ptr != NULL)
    {
        while (ptr != NULL)
        {
            if (((char *)(ptr + 1))[0] == '\"')
            {
                closeStrGap(ptr, (ptr + 1));
            }

            ptr++;

            ptr = strstr(ptr, "\"");
        }
    }
}

/* takes a string and strips off double quotes from either end */
void stripFieldQuotes(char *s)
{

    if (s[0] == '\"') // check if first char is dq
    {
        closeStrGap(s, (char *)(s + 1));
    }

    int len = strlen(s); // length of s

    if (s[len - 1] == '\"') // check if last char is dq, march back
    {
        s[len - 1] = '\0';
    }
}

/* takes 2 char* and moves  */
void closeStrGap(char *dest, char *src)
{
    int span = strlen(src) + 1; // calculate the span to be moved
    memmove(dest, src, span);   // memmove from src to dest for span bytes
}

/* check new card against cards
    return -1 if no match
    return -2 if match found but new card is obs
    return index if match found and new card supersedes
 */
int checkDuplicateCard(CARD *newCard, CARDARR *cards)
{

    for (size_t i = 0; i < cards->count; i++) // loop through cards
    {
        if (strcmp(newCard->name, cards->arr[i]->name) == 0) // compare newCard.name with cards[i].name
        {
            if (newCard->id <= cards->arr[i]->id) // check if newCard id is obs
            {
                return -2; // return obs flag
            }
            else // new card supersedes
            {
                return i; // return index of obs card
            }
        } // no match found, continue looping
    }
    return -1; // no match was found in cards
}

/* custom comparison function for Qsort
    returns strcmp of card names
 */
int comparCardName(const void *cardA, const void *cardB)
{
    const CARD *A = *(CARD **)cardA;
    const CARD *B = *(CARD **)cardB;
    return strcmp(A->name, B->name);
}

/* build a CARD from the constituent fields
    returns ptr to new CARD
    * memory is allocated each time called, must be freed
 */
CARD *cardBuilder(unsigned id, char *name, char *cost, unsigned converted_cost, char *type, char *text, char *stats, RARITY rarity)
{

    CARD *card = malloc(sizeof(CARD)); // allocate space for one card

    if ((id != 0) && // check that id is not blank
        (name)       // check that name pointer is not null
    )
    {
        card->id = id;
        card->name = name;
        card->cost = cost;
        card->converted_cost = converted_cost;
        card->type = type;
        card->text = text;
        card->stats = stats;
        card->rarity = rarity;
    }
    else // id is 0, or name is null
    {
        fprintf(stderr, "\nError: ID/Name field invalid: %d, %s\n", id, name);
        free(card);
        return NULL;
    }
    return card;
}

/* converts char* to RARITY
    returns RARITY
 */
RARITY strToRARITY(char *rarityStr)
{
    if (rarityStr) // check if rarityStr is null
    {
        // match the input to string of rarity, then put corresponding rarity into card
        for (size_t i = 0; i < rarityStrArrLen; i++)
        {
            if (strcmp(rarityStr, (void *)rarityStrArr[i]) == 0)
            {
                return (RARITY)i;
            }
        }
        // rarityStr did not match any entries in rarityStrArr[]
        fprintf(stderr, "Error: strToRarity: invalid str: %s\n", rarityStr);
        return 0;
    }
    else // rarityStr null
    {
        fprintf(stderr, "Error: strToRarity: str null\n");
        return 0;
    }
}

/* close gap in cards
    free spot being removed
    memmove to close gap
 */
void closeCardGap(CARDARR *cards, int index)
{
    freeCard(cards->arr[index]);
    memmove(
        &cards->arr[index],
        &cards->arr[index + 1],
        (sizeof(CARD *) * (cards->count - index - 1)));
        cards->count--;
}

/* prints entire cards array */
void printCards(CARDARR *cards)
{
    for (size_t i = 0; i < cards->count; i++)
    {
        printCard(cards->arr[i]);
    }
}

/* print a card in req'd format */
void printCard(CARD *card)
{
    fprintf(stdout, "%-32s%20s\n%-44s%8s\n%s\n%s\n%s\n%52s\n\n",
            card->name,
            card->cost,
            card->type,
            rarityStrArr[(int)card->rarity],
            hline,
            card->text,
            hline,
            card->stats);
}
// #endregion

/* Details from eLearning
## Instructions:
For this assignment, create a program called parser that takes the included .csv file and outputs a sorted list of entries to the screen.

1. Parse each line (entry) in the file into a card struct. See the included header file for a definition. Put this into a card.h file and include it in your code file!
2. When encountering a duplicate (a card with the same name as a card already read), you must check the card id field and keep the card with the highest id value!
3. Once the entire dataset is in memory, sort them by card name using your favorite sorting method (hint-hint Qsort). The output width should be 52 characters, the description is not bounded by this.
4. Output the sorted entries to stdout. 

## CSV Pitfalls
There are some things to keep in mind with the included dataset:

There are some duplicate cards (they have the exact same name). Do not add them when parsing.
Some cards are missing the stats field and thus have two consecutive delimeters (,,). The function strtok() does not handle this properly, write your own parser!
Two fields are not used in the output, but should still be parsed and included in the card struct for all entries.
Notice that each field in the .csv that is text begins and ends with a quote (id and converted_cost do not, they are numbers), but that isn't shown in the output. Get rid of them!
Also notice that the card's text field sometimes has two double-quotes. This is the proper way to encode a literal double-quote in a .csv file. When outputting to a file, there should only be one shown!
Finally, any actual new line in the card's text field is shown with the actual \ (backslash) and n characters. You must turn these into a '\n' (newline) character!
When pretty-printing remember that the stats field might be missing, in that case, the row that would should the stat field should be filled with spaces.

## Makefile
Remember, you must write your own makefile, such that when your repository is cloned, one only has to run make in the folder to produce the parser application. Also write a clean target that removes binaries or .o object files, so that we may make your program from scratch easily.

The program must be compiled with -std=gnu11, -Werror, and -Wall

## First Example:
$ make && ./parser short-cards-1.csv

Corpse Knight                                 {W}{B}
Creature - Zombie Knight                    uncommon
----------------------------------------------------
Whenever another creature enters the battlefield under your control, each opponent loses 1 life.
----------------------------------------------------
                                                 2/2

Eternal Isolation                             {1}{W}
Sorcery                                     uncommon
----------------------------------------------------
Put target creature with power 4 or greater on the bottom of its owner's library.
----------------------------------------------------
                                                    

Orzhov Enforcer                               {1}{B}
Creature - Human Rogue                      uncommon
----------------------------------------------------
Deathtouch
Afterlife 1 (When this creature dies, create a 1/1 white and black Spirit creature token with flying.)
----------------------------------------------------
                                                 1/2

Stolen by the Fae                          {X}{U}{U}
Sorcery                                         rare
----------------------------------------------------
Return target creature with converted mana cost X to its owner's hand. You create X 1/1 blue Faerie creature tokens with flying.
----------------------------------------------------
                                                    

$ cat short-cards-1.csv

"id","name","cost","converted_cost","type","text","stats","rarity"
198981,"Stolen by the Fae","{X}{U}{U}",2,"Sorcery","Return target creature with converted mana cost X to its owner's hand. You create X 1/1 blue Faerie creature tokens with flying.",,"rare"
192783,"Eternal Isolation","{1}{W}",2,"Sorcery","Put target creature with power 4 or greater on the bottom of its owner's library.",,"uncommon"
192783,"Eternal Isolation","{1}{W}",2,"Sorcery","Put target creature with power 4 or greater on the bottom of its owner's library.",,"uncommon"
192504,"Corpse Knight","{W}{B}",2,"Creature - Zombie Knight","Whenever another creature enters the battlefield under your control, each opponent loses 1 life.","2/2","uncommon"
183427,"Orzhov Enforcer","{1}{B}",2,"Creature - Human Rogue","Deathtouch\nAfterlife 1 (When this creature dies, create a 1/1 white and black Spirit creature token with flying.)","1/2","uncommon"

## Second Example:
$ make && ./parser short-cards-2.csv

Bomrek of the Thornhand                          {4}
Performance - Poet                          uncommon
----------------------------------------------------
He thinks that the world should be engaged in
perpetual warfare. He has a great deal of respect
for worthy crafts[man]ship. He doesn't really value
merrymaking. He sees working hard as a foolish waste
of time. He does not respect the law. He dislikes
cooperation. He doesn't have any strong feelings
about tradition. He needs alcohol to get through the
working day
----------------------------------------------------
                                                 7/2

Ilral                                      {3}{G}{G}
Engineer - Siege operator                     mythic
----------------------------------------------------
She finds artwork boring. She is offended by leisure
time and leisurely living. She sees cooperation as
very important in life. She sees equal parts of
harmony and discord as part of life. She doesn't see
much value in being stoic. She sees competition as
reasonably important. She thinks that introspection
is valueless and those that waste time in self-
examination are deluded fools. She needs alcohol to
get through the working day
----------------------------------------------------
                                                 5/7

Mafol Rockcrusher                       {X}{2}{U}{U}
Ranger - Trapper                                rare
----------------------------------------------------
She finds friendship burdensome. She is an absolute
believer in the rule of law. She sees guile and
cunning as indirect and somewhat worthless. She
doesn't value eloquence so much. She values hard
work. She doesn't particularly care between war and
peace. She finds romance distasteful. She needs
alcohol to get through the working day
----------------------------------------------------
                                                 4/4

Onol Waranvil                              {T}{4}{U}
Other Jobs - Alchemist                          rare
----------------------------------------------------
She values independence. She holds the view that
commerce is a vile obscenity. She finds friendship
burdensome. She holds crafts[man]ship to be of the
highest ideals and celebrates talented artisans and
their masterworks. She feels that introspection and
all forms of self-examination are the keys to a good
life and worthy of respect. She believes that the
acquisition of power over others is the ideal goal
in life and worthy of the highest respect. She finds
eloquence and artful speech off-putting. She needs
alcohol to get through the working day
----------------------------------------------------
                                                 2/2

$ cat short-cards-2.csv

"id","name","cost","converted_cost","type","text","stats","rarity"
4262949024,"Bomrek of the Thornhand","{4}",4,"Performance - Poet","He thinks that the world should be engaged in\nperpetual warfare. He has a great deal of respect\nfor worthy crafts[man]ship. He doesn't really value\nmerrymaking. He sees working hard as a foolish waste\nof time. He does not respect the law. He dislikes\ncooperation. He doesn't have any strong feelings\nabout tradition. He needs alcohol to get through the\nworking day","7/2","uncommon"
3411778384,"Ilral","{3}{G}{G}",6,"Engineer - Siege operator","She finds artwork boring. She is offended by leisure\ntime and leisurely living. She sees cooperation as\nvery important in life. She sees equal parts of\nharmony and discord as part of life. She doesn't see\nmuch value in being stoic. She sees competition as\nreasonably important. She thinks that introspection\nis valueless and those that waste time in self-\nexamination are deluded fools. She needs alcohol to\nget through the working day","5/7","mythic"
1679258629,"Mafol Rockcrusher","{X}{2}{U}{U}",1,"Ranger - Trapper","She finds friendship burdensome. She is an absolute\nbeliever in the rule of law. She sees guile and\ncunning as indirect and somewhat worthless. She\ndoesn't value eloquence so much. She values hard\nwork. She doesn't particularly care between war and\npeace. She finds romance distasteful. She needs\nalcohol to get through the working day","4/4","rare"
1983949280,"Onol Waranvil","{T}{4}{U}",4,"Other Jobs - Alchemist","She values independence. She holds the view that\ncommerce is a vile obscenity. She finds friendship\nburdensome. She holds crafts[man]ship to be of the\nhighest ideals and celebrates talented artisans and\ntheir masterworks. She feels that introspection and\nall forms of self-examination are the keys to a good\nlife and worthy of respect. She believes that the\nacquisition of power over others is the ideal goal\nin life and worthy of the highest respect. She finds\neloquence and artful speech off-putting. She needs\nalcohol to get through the working day","2/2","rare"

## Third Example 
(in the case that the input file does not exist)
$ make && ./parser does-not-exist.csv 1> /dev/null

(notice this output is from stderr, not stdout, there should be nothing printed on stdout)
Also, the exit-code of the program must be the integer value 1.

./parser: cannot open("does-not-exist.csv"): No such file or directory
$ echo $?
*/