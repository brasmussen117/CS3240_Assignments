#include <stdlib.h>

typedef struct node
{
	void *value;
	struct node *next;
} NODE;

typedef NODE *STACK;

bool push(STACK *stack, void *value) {
	NODE *newnode = malloc(sizeof(NODE));
	if (newnode == NULL) return false;
	
	newnode->value = value;
	newnode->next = *stack;

	*stack = newnode;
	return true;
}

void *pop(STACK *stack) {
	if (stack == NULL) return NULL;

	void *result = (*stack)->value;
	NODE *tmp = *stack;
	*stack = (*stack)->next;

	free(tmp);
	return result;
}

/* 
int main(){
	stack s1 = NULL, s2 = NULL, s3 = NULL;
	push(&s1, value1);
	push(&s2, value2);
	push(&s3, value3);
}
*/