/* 
	author: Brenden Rasmusen
	class: CS3240, Spring 2021

	content taken from Professor MacCreery's lecture streams 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define NAMELEN 20

int comparNames(const void *, const void *); // compare STUDENT->name

typedef struct student
{
	char *name;
	float gpa;
	uint32_t age;
	char *lgrade;
} STUDENT;

char *lgradeArray[] = {
	"A",
	"BA",
	"B",
	"CB",
	"C",
	"DC",
	"D",
	"E"
};

int main(int argc, char const *argv[])
{
	FILE *input = fopen("input.bin", "rb"); // open bin file
	
	uint32_t *nrecords = malloc(sizeof(uint32_t)); // number of records
	fread(nrecords, sizeof(uint32_t), 1, input); // get nrecords from input
	uint32_t count = nrecords[0]; // dereference for simplicity
	free(nrecords); // free nrecords, not needed

	STUDENT **students = malloc(sizeof(STUDENT*) * count); // ptr[] of students

	char *_name = malloc(NAMELEN); // tmp var for pulling entire 20 bytes from input

	/* loop to alloc each student, read bin, and fill out student */
	for (size_t i = 0; i < count; i++)
	{
		students[i] = malloc(sizeof(STUDENT));
		
		fread(_name, NAMELEN, 1, input);
		students[i]->name = strndup(_name, strlen(_name));
		
		fread(&students[i]->gpa, sizeof(float), 1, input);
		fread(&students[i]->age, sizeof(uint32_t), 1, input);
	}
	free(_name); // free _name, not needed after loop

	/* close the file */
	if (fclose(input) != 0)
	{
		printf("Error: file failed to close");
		return -1;
	}

	/* calculate lgrade */
	float sw = 0;
	int index = 0;
	for (size_t i = 0; i < count; i++)
	{
		sw = students[i]->gpa;
		
		     if (sw == 4.0) { index = 0; }
		else if (sw >= 3.5) { index = 1; }
		else if (sw >= 3.0) { index = 2; }
		else if (sw >= 2.5) { index = 3; }
		else if (sw >= 2.0) { index = 4; }
		else if (sw >= 1.5) { index = 5; }
		else if (sw >= 1) 	{ index = 6; }
		else 				{ index = 7; }

		students[i]->lgrade = lgradeArray[index];
	}
	
	// TODO: sort students by name
	qsort(students, count, sizeof(STUDENT*), comparNames);

	/* print students */
	for (size_t i = 0; i < count; i++)
	{
		printf("%lu of %u:\nName: %s\nGPA:  %.1f (%s)\nAge:  %u\n\n", (i + 1), count, students[i]->name, students[i]->gpa, students[i]->lgrade, students[i]->age);
	}

	/* free memory */
	for (size_t i = 0; i < count; i++)
	{
		free(students[i]->name);
		free(students[i]);
	}
	free(students);

	return 0;
}

/* compare STUDENT->name
	syntax help from StackOverflow
		https://stackoverflow.com/questions/49735555/qsort-with-structs-in-c
			contributor: https://stackoverflow.com/users/9493582/arndt-jonasson
		https://stackoverflow.com/questions/6105513/need-help-using-qsort-with-an-array-of-structs
			contributor: https://stackoverflow.com/users/1973415/koko-auth
 */
int comparNames(const void *studentA, const void *studentB){
	const STUDENT *A = *(STUDENT **)studentA;
	const STUDENT *B = *(STUDENT **)studentB;
	return strcmp(A->name, B->name);
}