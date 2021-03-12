#include "card.h"
#include "useful_stuff.h"

/* function prototypes */

int checkDuplicateCard(CARD *, CARD **, size_t);
int comparCardName(const void *, const void *);
CARD *cardBuilder(unsigned, char *, char *, unsigned, char *, char *, char *, RARITY);
RARITY strToRARITY(char *);
void closeGap(CARD **, int, int);
char *convNewlineChar(char *);
void printCard(CARD *);
void printCards(CARD **, int);
void freeCard(CARD *);
void freeCards(CARD **, int);
CARD *parse_csv(char *);
char *stripQuotes(char *);



int main(int argc, char const *argv[])
{
	/* check for necessary arguments --------------------------- */
    if (argc != 2)
    {
        printf("\nUsage: ./parser <file_name>\n\n");
        return -1;
    }

    /* open file ----------------------------------------------- */
    FILE *input_file = fopen(argv[1], "r"); // open the file given as argument in command line

    if (input_file == NULL) // check that fopen was sucessful
    {
        printf("Error: file not succesfully opened");
        return -2;
    }
    
    /* setup and get first line -------------------------------- */
    char *buf = NULL; // buffer
    size_t n = 0; // size of result

    ssize_t result = getline(&buf, &n, input_file); // read the first line

    if (result < 0) // check that the file is not empty
    {
        printf("Error: file emtpy or getline error\n");
        return 1;
    }

    result = getline(&buf, &n, input_file); // read the second line

    CARD **cards = NULL; // array of CARD*
    CARD *newCard = NULL;

    /* read loop ------------------------------------------------ */
    
    ssize_t dup = 0; // duplicate card flag

    size_t count = 0; // main counter

    while (result > 0) // primary read/parse loop
    {
        newCard = parse_csv(buf);

        /* check for and handle duplicates */
        dup = checkDuplicateCard(newCard, cards, count); // check for duplicate
        if (dup != -1) // if dup returns a match then handle it
        {
            if (dup == -2) // check if new entry is not superseding, skip this entry if not
            {
                freeCard(newCard);
                result = getline(&buf, &n, input_file); // read the next line
                continue; // skip to next iteration
            } else // new entry IS superseding, dup has index of obs card
            {
                closeGap(cards, dup, count--); // close the gap to remove obs card
            }
        } // else no duplicate found, then proceed

        cards = realloc(cards, sizeof(CARD*) * ++count); // realloc
        
        cards[count - 1] = newCard; // put newCard into cards, increment count

        result = getline(&buf, &n, input_file); // read the next line
    } // end read loop

    qsort(cards, count, sizeof(CARD*), comparCardName); // sort cards by name

    printCards(cards, count); // print cards

    if (fclose(input_file) != 0) // close the file
    {
        printf("Error: file not successfully closed\n");
        return -2;
    }
    
    /* free memory */
    freeCards(cards, count);
    
	return 0;
}

/* frees a CARD* and char* fields */
void freeCard(CARD *cardtofree){
    /* free the cards's char* fields */
    free(cardtofree->name);
    free(cardtofree->cost);
    free(cardtofree->type);
    free(cardtofree->text);
    free(cardtofree->stats);
    
    free(cardtofree); // free the card
}

/* frees full CARD** array */
void freeCards(CARD **cards, int count){
    /* loop to free each of the cards */
    for (size_t i = 0; i < count; i++)
    {
        freeCard(cards[i]);
    }
    free(cards); // free the full array
}

/* take buf line input, returns pointer to new card
 */
CARD *parse_csv(char *buf){
    unsigned int _id = 0;               // var for bulding new card
    char *_name = NULL;                 // var for bulding new card
    char *_cost = NULL;                 // var for bulding new card
	unsigned int _converted_cost = 0;   // var for bulding new card
	char *_type = NULL;                 // var for bulding new card
	char *_text = NULL;                 // var for bulding new card
	char *_stats = NULL;                // var for bulding new card
    RARITY _rarity;                     // var for bulding new card

    char *line = strdup(buf); // dup the buf for strsep
    char *free_line = line; // pointer for cleaning memory in loop
    char *rarityStr = NULL; // take rarity as string for conversion
    char *temp_text = NULL; // place holder for stripping quotes
    char *comma = NULL; // ptr for examining comma postions
    
    ssize_t offset = 0; // ptr for str separating

    /* begin parsing input ------------------------------------------- */

    _id = atoi(strsep(&line, ",")); // get id field, convert to int

    comma = strstr(line, ","); // find position of next comma
    if (
        (((char *)(comma - 1))[0] == '\"') && // check if there are double quotes on either side of comma
        (((char *)(comma + 1))[0] == '\"')
        )
    { // comma is between double quotes
        _name = stripQuotes(strsep(&line, ",")); // get name field, strip quotes
    }
    else // comma was not between double quotes, presume comma inside name field
    {
        line++; // advance past the beginning dq
        _name = stripQuotes(strsep(&line, "\"")); // get name field, strip quotes
        line++; // advance past the comma
    }
        
    _cost = stripQuotes(strsep(&line, ",")); // get cost field, strip quotes
    
    _converted_cost = atoi(strsep(&line, ",")); // get converted_cost field, convert to int

    _type = stripQuotes(strsep(&line, ",")); // get type field, strip quotes

    /* backtrack from the end to get text field ---------------------- */

    offset = (strlen(line)); // set offset to the end of line, pre-increment past first quote

    while (line[offset] != ','){ // march back to the comma before rarity
        offset--;
    }

    offset--; // set before last comma

    while (line[offset] != ','){ // march offset to the comma before stats
        offset--;
    }

    temp_text = strndup(line, offset); // dup the text field
    
    _text = stripQuotes(temp_text);

    free(temp_text);

    _text = convNewlineChar(_text); // manage the \n chars

    line = line + offset + 1; // point to first char of stat field

    /* strsep the remainder as before -------------------------------- */

    _stats = stripQuotes(strsep(&line, ",")); // get stats field, strip quotes

    if (strstr(line,"\r"))
    {
        rarityStr = stripQuotes(strsep(&line, "\r")); // get rarityStr, strip quotes
    } else
    {
        rarityStr = stripQuotes(strsep(&line, "\n")); // get rarityStr, strip quotes
    }
    
    _rarity = strToRARITY(rarityStr); // convert rarityStr to RARITY

    free(rarityStr); // free the string

    /* clean up, build the new card, and return ---------------------- */

    free(free_line); // free memory

    return cardBuilder(
        _id,
        _name,
        _cost,
        _converted_cost,
        _type,
        _text,
        _stats,
        _rarity
    );
}

/* takes a string and strips off double quotes from either end
    return s if it's empty
    return allocated string if not empty
        memory must be freed
 */
char *stripQuotes(char *s){
    int len = strlen(s);

    if (len != 0) // check if s is not empty
    {
        int end = len - 2; // point at last char, before \"

        if (s[0] == '\"')  // check if first char is dq
        { 
            s++; // advance past dq
            len--; // decrement for lost char
        }
        
        if (s[end] == '\"')  // check if last char is dq, march back
        { 
            len--; // decrement before dq
        }

        return strndup(s, len); // dup s from index 1 to index end-1
    }
    return strdup("\0"); // s was empty, retutn new pointer to zero
}

/* check new card against cards
    return -1 if no match
    return -2 if match found but new card is obs
    return index if match found and new card supersedes
 */
int checkDuplicateCard(CARD *newCard, CARD **cards, size_t count){

    for (size_t i = 0; i < count; i++) // loop through cards
    {
        if (strcmp(newCard->name, cards[i]->name) == 0) // compare newCard.name with cards[i].name
        {
            if (newCard->id <= cards[i]->id) // check if newCard id is obs
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
int comparCardName(const void *cardA, const void *cardB){
    const CARD *A = *(CARD **)cardA;
    const CARD *B = *(CARD **)cardB;
    return strcmp(A->name, B->name);
}

/* build a CARD from the constituent fields
    returns ptr to new CARD
    * memory is allocated each time called, must be freed
 */
CARD* cardBuilder(unsigned id, char* name, char* cost, unsigned converted_cost, char* type, char* text, char* stats, RARITY rarity){
    
    CARD* result = malloc(sizeof(CARD)); // allocate space for one card

    if ((id != 0) && // check that id is not blank
        (name) // check that name pointer is not null
    )
    {
        result->id = id;
        result->name = name;
        result->cost = cost;
        result->converted_cost = converted_cost;
        result->type = type;
        result->text = text;
        result->stats = stats;
        result->rarity = rarity;
    }
    else // id is 0, or name is null
    {
        printf("\nError: ID/Name field invalid");
        printf("\nID: %d, Name: %s", id, name);
        return NULL;
    }
    return result;
}

/* converts char* to RARITY
    returns RARITY
 */
RARITY strToRARITY(char *rarityStr){
    if (rarityStr) // check if rarityStr is null
    {
        // match the input to string of rarity, then put corresponding rarity into card
        for (size_t j = 0; j < ARRAY_SIZE(rarityStringArray); j++)
        {
            if (strcmp(rarityStr, (void *)rarityStringArray[j]) == 0)
            {
                return (RARITY)j;
            }
        }
        // rarityStr did not match any entries in rarityStringArray[]
        printf("Error: strToRarity: invalid str: %s\n",rarityStr);
        return 0;
    }
    else // rarityStr null
    {
        printf("Error: strToRarity: str null\n");
        return 0;
    }
}

/* close gap in cards
    free spot being removed
    memmove to close gap
 */
void closeGap(CARD** cards, int index, int count){
    freeCard(cards[index]);
    memmove(&cards[index], &cards[index + 1], sizeof(CARD *) * (count - index));
}

/* take char*, find "\n" as string, and convert to special character
    returns new pointer
    previous pointer freed
 */
char* convNewlineChar(char* _text){
    char *result = malloc(strlen(_text));

    size_t new = 0; // counter for the result
    size_t old = 0; // counter for the input

    size_t loop = strlen(_text); // loop counter

    while (loop > 0)
    {
        if ((_text[old] == '\\') && // check each char for '\'
            (_text[old + 1] == 'n')) // if '\' found check next char for n
        { // \n found
            result[new++] = '\n'; // send \n into result
            old += 2; // advance old past n
            loop--; // extra decrement for squashing 2 chars
        } else{ // not \n
            result[new++] = _text[old++]; // send old char to result
        }
        loop--;
    }

    if (new == old) // check if there were no \n in _text
    {
        free(result); // free the unneeded alloc
        return _text; // return arg without change
    } else // there was \n in _text, need to realloc for the size difference
    {
        result[new] = 0; // null terminate the string
        free(_text); // free the input alloc
        char *ret = realloc(result, strlen(result));
        if (ret != result) // check if realloc made a new alloc
        {
            free(result); // free the old alloc
        }
        return ret;
    }
}

/* prints entier cards array */
void printCards(CARD** cards, int count){
    for (size_t i = 0; i < count; i++)
    {
        printCard(cards[i]);
    }
}

/* print a card in req'd format */
void printCard(CARD *card){
    // TODO: print cards in correct format
        printf(" id: %u; name: %s; cost: %s; conv_cost: %u; type: %s; stats: %s; rarity: %s;\n text: \n%s\n\n",
        card->id,
        card->name,
        card->cost,
        card->converted_cost,
        card->type,
        card->stats,
        rarityStringArray[(int)card->rarity],
        card->text
    );
}

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