/* required enum and struct */

typedef enum rarity
{
	common,
	uncommon,
	rare,
	mythic
} RARITY;

typedef struct card
{
	unsigned int id;
	char* name;
	char* cost;
	unsigned int converted_cost;
	char* type;
	char* text;
	char* stats;
	RARITY rarity;
} CARD;
