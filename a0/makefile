main: main.c
	gcc -std=gnu11 -Werror -Wall -o main main.c
clean:
	rm -f main
debug:
	gcc -std=gnu11 -Werror -Wall -o main main.c -g
valgrind:
	valgrind --tool=memcheck --leak-check=full ./main input3.txt
