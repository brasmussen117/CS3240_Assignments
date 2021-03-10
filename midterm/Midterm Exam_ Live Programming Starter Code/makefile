make:
	cc -std=gnu11 -Wall -Werror -o main main.c

debug:
	cc -std=gnu11 -Wall -Werror -o main main.c -g

run: main
	./main

clean: main
	rm main

gdb: main.c
	cc -std=gnu11 -Wall -Werror -o main main.c -g
	gdb main

valgrind:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./main