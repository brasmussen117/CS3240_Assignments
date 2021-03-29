#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

/* defs */
#define cardbinfn ".cardbin"
#define indexbinfn ".indexbin"

/* structs --------------------------------------------------------- */
typedef struct cardarr // struct for holding array of cards and the size
{
	CARD **arr;
	size_t size;
} CARDARR;

typedef struct index // struct for index entry values
{
    char *name;
    long *offset;
} INDEX;

typedef struct indexarr
{
    INDEX **arr;
    uint32_t *size;
} INDEXARR;


const char* rarityStrArr[] = { // array for string values of RARITY enum
	"common",
	"uncommon",
	"rare",
	"mythic"
};

/* global vars ----------------------------------------------------- */

const int rarityStrArrLen = 4; // size of rarityStrArr[]


/* horizontal border */
const char *hline = "----------------------------------------------------";

char *zptr_char = "\0"; // a single reusable pointer for holding a zero char

/* frees a CARD* and char* fields
    checks if certain fields are pointing to zptr_char:
        cost, text, stats
 */
void freeCard(CARD *cardtofree)
{
    /* free the cards's char* fields */
    free(cardtofree->name);
    if (cardtofree->cost != zptr_char)
    {
        free(cardtofree->cost);
    }
    free(cardtofree->type);
    if (cardtofree->text != zptr_char)
    {
        free(cardtofree->text);
    }
    if (cardtofree->stats != zptr_char)
    {
        free(cardtofree->stats);
    }

    free(cardtofree); // free the card
}

/* frees full CARD** array */
void freeCards(CARDARR *cards)
{
    /* loop to free each of the cards */
    for (size_t i = 0; i < cards->size; i++)
    {
        freeCard(cards->arr[i]);
    }
    free(cards->arr); // free the full array
    free(cards);
}

/* takes 2 char* and moves  */
void closeStrGap(char *dest, char *src)
{
    int span = strlen(src) + 1; // calculate the span to be moved
    memmove(dest, src, span);   // memmove from src to dest for span bytes
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

/* take an allocated string and clean as needed
    checks if null, return zptr_char
    calls convNewlineChar, stripFieldQuotes, stripDoubledQuotes as needed
    returns realloc'd ptr
 */
char *cleanstr(char *s)
{
    if ((s != zptr_char) && (s[0] != '\0'))
    {
        stripFieldQuotes(s); // strip the quotes from beginning/end

        stripDoubledQuotes(s); // strip doubled double quotes ("") to be a single double quote

        convNewlineChar(s); // strip characters '\' & 'n' and places special \n character

        return s;
    }
    else // field was empty
    {
        free(s); // free the allocated input

        return zptr_char; // return pointer to zero
    }
}

/* check new card against cards
    return -1 if no match
    return -2 if match found but new card is obs
    return index if match found and new card supersedes
 */
int checkDuplicateCard(CARD *newCard, CARDARR *cards)
{

    for (size_t i = 0; i < cards->size; i++) // loop through cards
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
        (sizeof(CARD *) * (cards->size - index - 1)));
        cards->size--;
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

/* prints entire cards array */
void printCards(CARDARR *cards)
{
    for (size_t i = 0; i < cards->size; i++)
    {
        printCard(cards->arr[i]);
    }
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

/* loop and read file, puts cards into *CARDARR 
    return *CARDARR on success
    return NULL on failure
*/
CARDARR *parse_file_csv(FILE *input_file)
{
    CARDARR *cards = malloc(sizeof(CARDARR));
    cards->arr = NULL;
    cards->size = 0;

    /* setup and get first line -------------------------------- */
    char *buf = NULL; // buffer
    size_t n = 0;     // size of line

    ssize_t linesize = getline(&buf, &n, input_file); // read the first line, header unused

    if (linesize < 0) // check that the file is not empty
    {
        fprintf(stderr, "Error->parse_file_csv: file emtpy or getline error\n");
        fclose(input_file);
        free(cards);
        return NULL;
    }

    CARD *newCard = NULL; // pointer for a new card

    ssize_t dup = 0; // duplicate card flag

    while (linesize > 0) // read/parse loop
    {
        linesize = getline(&buf, &n, input_file); // read the next line

        if (linesize <= 0) // check if there was a new line
        {
            break; // break if line empty or error
        }

        newCard = parse_line_csv(buf);

        /* check for and handle duplicates ------------------------- */
        if (cards->size > 0) // check if first loop
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

        cards->arr = realloc(cards->arr, sizeof(CARD *) * ++cards->size); // realloc cards, pre-increment size

        cards->arr[cards->size - 1] = newCard; // put newCard into cards
    } // end read loop

    free(buf);

    return cards;
}
