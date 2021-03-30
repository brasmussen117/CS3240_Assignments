#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

/* defs */
#define cardbinfn "bin/cards.bin"
#define indexbinfn "bin/index.bin"
#define TERMINAL 1 // isatty(STDIN_FILENO) == 1
#define REDIRECT 0 // isatty(STDIN_FILENO) == 0

const char *rarityStrArr[] = { // array for string values of RARITY enum
    "common",
    "uncommon",
    "rare",
    "mythic"};

/* #region structs ------------------------------------------------- */
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
/* #endregion */

/* #region global vars --------------------------------------------- */

const int rarityStrArrLen = 4; // size of rarityStrArr[]

/* horizontal border */
const char *hline = "----------------------------------------------------";

char *zptr_char = "\0"; // a single reusable pointer for holding a zero char
/* #endregion */

/* #region  FUNCTIONS ---------------------------------------------- */

/* #region helper functions */

/* convert uint32_t to ptr */
uint32_t *convtoptr_uint32_t(uint32_t i)
{
    uint32_t *ret = malloc(sizeof(uint32_t));
    *ret = i;
    return ret;
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

/* #endregion */

/* #region  index funcs */

/* create INDEX* from fields */
INDEX *indexbuilder(char *name, long offset)
{
    INDEX *newentry = malloc(sizeof(INDEX));

    newentry->offset = malloc(sizeof(long));
    *newentry->offset = offset;
    newentry->name = name;

    return newentry;
}

/* free INDEXARR *indices */
void freeindices(INDEXARR *indices)
{
    for (size_t i = 0; i < *indices->size; i++)
    {
        free(indices->arr[i]->offset);
        free(indices->arr[i]);
    }
    free(indices->arr);
    free(indices->size);
    free(indices);
}
/* #endregion */

/* #region parser funcs */

/* write cards to bin file */
INDEXARR *writeCardBin(FILE *output_card_file, CARDARR *cards)
{
    INDEXARR *indices = malloc(sizeof(INDEXARR));
    indices->arr = malloc(sizeof(INDEX*) * cards->size);
    indices->size = malloc(sizeof(uint32_t));
    *indices->size = (uint32_t)cards->size;

    for (size_t i = 0; i < cards->size; i++)
    {
        /* make the index entries ---------------------------------- */
        indices->arr[i] = indexbuilder(cards->arr[i]->name, ftell(output_card_file));

        // #region write card fields -------------------------------

        // #region id - unsigned int
        uint32_t *idout = convtoptr_uint32_t((uint32_t)cards->arr[i]->id);
        if (!fwrite(idout,
                    sizeof(uint32_t),
                    1,
                    output_card_file))
        {
            fprintf(stderr, "parser::writeCardBin: id field not written: %d\n", cards->arr[i]->id);
            exit(EXIT_FAILURE);
        }
        free(idout);
        // #endregion

        // #region cost - char*
        uint32_t *cost_len = convtoptr_uint32_t((uint32_t)strlen(cards->arr[i]->cost));
        if (!fwrite(cost_len,
                    sizeof(uint32_t),
                    1,
                    output_card_file))
        {
            fprintf(stderr, "parser::writeCardBin: cost size not written: %s\n", cards->arr[i]->cost);
            exit(EXIT_FAILURE);
        }

        if (cost_len > 0) // check if field was blank
        {
            if (
                fwrite(cards->arr[i]->cost,
                       sizeof(char),
                       *cost_len,
                       output_card_file) != *cost_len)
            {
                perror("parser::writeCardBin: cost field not written");
                exit(EXIT_FAILURE);
            }
        }
        free(cost_len);
        // #endregion

        // #region converted_cost - unsigned int
        uint32_t *converted_cost_out = convtoptr_uint32_t((uint32_t)cards->arr[i]->converted_cost);
        if (!fwrite(converted_cost_out,
                    sizeof(uint32_t),
                    1,
                    output_card_file))
        {
            fprintf(stderr, "parser::writeCardBin: converted_cost field not written: %d\n", cards->arr[i]->converted_cost);
            exit(EXIT_FAILURE);
        }
        free(converted_cost_out);
        // #endregion

        // #region type - char*
        uint32_t *type_len = convtoptr_uint32_t((uint32_t)strlen(cards->arr[i]->type));
        if (!fwrite(type_len,
                    sizeof(uint32_t),
                    1,
                    output_card_file))
        {
            fprintf(stderr, "parser::writeCardBin: type size not written: %s\n", cards->arr[i]->type);
            exit(EXIT_FAILURE);
        }
        free(type_len);

        if (!fwrite(cards->arr[i]->type,
                    sizeof(char),
                    strlen(cards->arr[i]->type),
                    output_card_file))
        {
            fprintf(stderr, "parser::writeCardBin: type field not written: %s\n", cards->arr[i]->type);
            exit(EXIT_FAILURE);
        }
        // #endregion

        // #region text - char*
        uint32_t *text_len = convtoptr_uint32_t((uint32_t)strlen(cards->arr[i]->text));
        if (!fwrite(text_len,
                    sizeof(uint32_t),
                    1,
                    output_card_file))
        {
            fprintf(stderr, "parser::writeCardBin: text size not written: %s\n", cards->arr[i]->text);
            exit(EXIT_FAILURE);
        }
        if (text_len > 0) // check if field was blank
        {
            if (!fwrite(cards->arr[i]->text,
                        sizeof(char),
                        strlen(cards->arr[i]->text),
                        output_card_file))
            {
                fprintf(stderr, "parser::writeCardBin: text field not written: %s\n", cards->arr[i]->text);
                exit(EXIT_FAILURE);
            }
        }
        free(text_len);
        // #endregion

        // #region stats - char*
        uint32_t *stats_len = NULL;
        if (cards->arr[i]->stats != zptr_char) // check if field was blank, if so remain zero
        {
            stats_len = convtoptr_uint32_t((uint32_t)strlen(cards->arr[i]->stats));
        } else
        {
            stats_len = convtoptr_uint32_t(0);
        }

        if (fwrite(stats_len,
                    sizeof(uint32_t),
                    1,
                    output_card_file) != 1) // try to write, handle if fail
        {
            fprintf(stderr, "parser::writeCardBin: stats size not written: %s\n", cards->arr[i]->stats);
            exit(EXIT_FAILURE);
        }

        if (stats_len > 0) // check if anything to write
        {
            if (fwrite(cards->arr[i]->stats,
                    sizeof(char),
                    *stats_len,
                    output_card_file) 
                != *stats_len) // try to write, handle if fail
            {
                perror("parser::writeCardBin: stats field not written");
                exit(EXIT_FAILURE);
            }
        }
        
        free(stats_len);
        // #endregion

        // #region rarity
        uint32_t *rarity_out = convtoptr_uint32_t((uint32_t)cards->arr[i]->rarity);
        if (!fwrite(rarity_out,
                    sizeof(uint32_t),
                    1,
                    output_card_file))
        {
            fprintf(stderr, "parser::writeCardBin: rarity field not written: %d\n", cards->arr[i]->rarity);
            exit(EXIT_FAILURE);
        }
        free(rarity_out);
        // #endregion rarity

        // #endregion write card fields
    
    }

    return indices;
}

/* write card index to bin file */
void writeIndexBin(FILE *output_index_file, INDEXARR *indices)
{
    /* write the size of the index - uint32_t ---------------------- */
    if (!fwrite(indices->size,
                sizeof(uint32_t),
                1,
                output_index_file))
    {
        fprintf(stderr, "parser::writeIndexBin: index size not written: %d\n", *indices->size);
        exit(EXIT_FAILURE);
    }

    /* loop and write each index entry ----------------------------- */
    for (size_t i = 0; i < *indices->size; i++)
    {
        /* name size - uint32_t */
        uint32_t *name_len = convtoptr_uint32_t((uint32_t)strlen(indices->arr[i]->name));
        if (!fwrite(name_len,
                    sizeof(uint32_t),
                    1,
                    output_index_file))
        {
            fprintf(stderr, "parser::writeIndexBin: name size not written: %s\n", indices->arr[i]->name);
            exit(EXIT_FAILURE);
        }
        free(name_len);

        /* name - char* */
        if (!fwrite(indices->arr[i]->name,
                    sizeof(char),
                    strlen(indices->arr[i]->name),
                    output_index_file))
        {
            fprintf(stderr, "parser::writeIndexBin: name field not written: %s\n", indices->arr[i]->name);
            exit(EXIT_FAILURE);
        }

        /* offset - long */
        if (!fwrite(indices->arr[i]->offset,
                    sizeof(long),
                    1,
                    output_index_file))
        {
            fprintf(stderr, "parser::writeIndexBin: offset field not written: %ln\n", indices->arr[i]->offset);
            exit(EXIT_FAILURE);
        }
    }
}


/* #endregion */

/* #region card funcs */
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
            if (dup != -1)                            // if dup returns a match then handle it
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
        }     // else first loop, proceed

        cards->arr = realloc(cards->arr, sizeof(CARD *) * ++cards->size); // realloc cards, pre-increment size

        cards->arr[cards->size - 1] = newCard; // put newCard into cards
    }                                          // end read loop

    free(buf);

    return cards;
}
/* #endregion */

/* #endregion FUNCTIONS */
