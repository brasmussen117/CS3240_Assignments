cc = cc
CFLAGS = -Wall -Werror

index = index.bin
cards = cards.bin

file = search

testfile1 = ../a1-2_cards_csv/cards.csv
testfile2 = ./helpers/cards.csv
testfile3 = ./s.csv

input_r = < input.txt
input = $(input_r)

stdout = stdout.txt
stderr = stderr.txt
valout = valout.txt

output = 1> $(stdout) 2> $(stderr)

make:
	cc -std=gnu11 -Wall -Werror -o parser parser.c
	cc -std=gnu11 -Wall -Werror -o search search.c

clean:
	rm -f ./parser
	rm -f ./search
ifeq ($(file), parser)
	rm -f ./cards.bin
	rm -f ./index.bin
endif

debug: $(file).c
	$(cc) -std=gnu11 $(CFLAGS) -o $(file) $(file).c -g -no-pie

gdb: $(file) debug
	gdb $(file)

hexdump: $(index) $(cards)
	hexdump -Cv $(index) > hexdump_$(index)
	hexdump -Cv $(cards) > hexdump_$(cards)

valgrind:
	rm -f $(stdout) $(stderr) $(valout)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --log-file="$(valout)" ./$(file) $(input) $(output)

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./$(file) $(input)
