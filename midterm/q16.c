#include <stdlib.h>
#include <string.h>

typedef struct tag {
	int *id;
	char name[4];
} tag_t;

const char *names[3] = { "Bob", "Amy", "Cam" };

int main(int argc, char **argv) {
	int *num = malloc(sizeof(int));
	*num = 234;

	tag_t e1;
	e1.id = num;
	strncpy(e1.name, names[1], 4);

	tag_t *employees[3];

	employees[1] = &e1;

	int i;
	for(i = 0; i < 3; ++i) {
		if(i == 1) continue;
		employees[i] = malloc(sizeof(tag_t));
		employees[i]->id = malloc(sizeof(int));
		employees[i]->id[0] = 900 + i;
		strncpy(employees[i]->name, names[i], 4);
	}

	employees[2]->id = &i;
	*employees[2]->id *= 301;

	return 0;
}