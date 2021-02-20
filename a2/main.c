/* specs from mimir 
# DESCRIPTION
For this assignment, you are to build off of the previous assignment's program which should make this much easier to handle.

Create two programs, one called `parser` as before that takes the `.csv` file and outputs a sorted index of entries to a binary file (`index.bin`) and the entries themselves to a separate binary file. The Parser should not write duplicate records to the binary files. The other program is called `search` and will open the index file to binary search a desired card by name repeatedly until the user quits. The card information is read from the separate entries file (`cards.bin`) directly.

# Parser
1. As before, parse each line in the file into a `CARD` struct. See the included header file for a definition.
2. Once the entire dataset is in memory, sort them by card name using the `qsort()` function.
3. In one binary file (`index.bin`), write the card name and offset in which the record begins in the other file. The name length should be an int (uint32_t), then the characters of the string, without the null character. Theses names and the offsets should be written in alphabetical order.
4. In the second binary file (`cards.bin`), write the rest of the card data, which should be everything but the name. Each string field of the struct should first have an integer (uint32_t) indicating the string length, then the characters, without a null termininator. These fields should be written in the same order as they were read in from the CSV file.

# Search
1. Reading the entire index file (`index.bin`). every entry should be placed into a struct and into an array.
2. It should already be sorted. Read user input and call `bsearch()` (binary search) for that card name.
3. If a match is found, read the byte offset that is associated with that card name. Read from the other file (`cards.bin`) to obtain the rest of the card data and assemble the `CARD` record. Print it to the screen.
4. Running the program and typing input in to the terminal is different than when input is redirected to the program. During input redirection input is not echoed out to the terminal. In order to present to the tester a view of your program closer to when it is executed normally, if the input is not from a terminal, you must echo the inputted line to `stdout`! This can be accomplished by calling the function `isatty(0)`, please read the manpage for this, but basically it will return a 1 if input is coming from a terminal, otherwise it will return 0 (and also set `errno`, but we don't need that information).

# Makefile
Remember, you must write your own `makefile`, such that when your repository is cloned, one only has to run `make` in the folder to produce the `parser` and `search` applications

# Example:
$ cat cards.csv

"id","name","cost","converted_cost","type","text","stats","rarity"
198981,"Stolen by the Fae","{X}{U}{U}",2,"Sorcery","Return target creature with converted mana cost X to its owner's hand. You create X 1/1 blue Faerie creature tokens with flying.",,"rare"
192783,"Eternal Isolation","{1}{W}",2,"Sorcery","Put target creature with power 4 or greater on the bottom of its owner's library.",,"uncommon"
192783,"Eternal Isolation","{1}{W}",2,"Sorcery","Put target creature with power 4 or greater on the bottom of its owner's library.",,"uncommon"
192504,"Corpse Knight","{W}{B}",2,"Creature - Zombie Knight","Whenever another creature enters the battlefield under your control, each opponent loses 1 life.","2/2","uncommon"
183427,"Orzhov Enforcer","{1}{B}",2,"Creature - Human Rogue","Deathtouch\nAfterlife 1 (When this creature dies, create a 1/1 white and black Spirit creature token with flying.)","1/2","uncommon"
$ make && ./parser cards.csv

(There is no output for this command)

$ hexdump -Cv index.bin

00000000  0d 00 00 00 43 6f 72 70  73 65 20 4b 6e 69 67 68 |....Corpse Knigh|
00000010  74 26 01 00 00 00 00 00  00 11 00 00 00 45 74 65 |t&...........Ete|
00000020  72 6e 61 6c 20 49 73 6f  6c 61 74 69 6f 6e ac 00 |rnal Isolation..|
00000030  00 00 00 00 00 00 0f 00  00 00 4f 72 7a 68 6f 76 |..........Orzhov|
00000040  20 45 6e 66 6f 72 63 65  72 c3 01 00 00 00 00 00 | Enforcer.......|
00000050  00 11 00 00 00 53 74 6f  6c 65 6e 20 62 79 20 74 |.....Stolen by t|
00000060  68 65 20 46 61 65 00 00  00 00 00 00 00 00       |he Fae........|
0000006e
note: hexdump does not produce colored output, that was added manually to help you understand the example better. Every uint32_t representing the card's name's string length, is colored DarkGoldenRod. Every char[] giving the card's name is colored DarkSalmon. Every off_t indicating the location of where in cards.bin the remaining card's fields can be found is colored DarkOliveGreen.

$ hexdump -Cv cards.bin

00000000  45 09 03 00 09 00 00 00  7b 58 7d 7b 55 7d 7b 55  |E.......{X}{U}{U|
00000010  7d 02 00 00 00 07 00 00  00 53 6f 72 63 65 72 79  |}........Sorcery|
00000020  80 00 00 00 52 65 74 75  72 6e 20 74 61 72 67 65  |....Return targe|
00000030  74 20 63 72 65 61 74 75  72 65 20 77 69 74 68 20  |t creature with |
00000040  63 6f 6e 76 65 72 74 65  64 20 6d 61 6e 61 20 63  |converted mana c|
00000050  6f 73 74 20 58 20 74 6f  20 69 74 73 20 6f 77 6e  |ost X to its own|
00000060  65 72 27 73 20 68 61 6e  64 2e 20 59 6f 75 20 63  |er's hand. You c|
00000070  72 65 61 74 65 20 58 20  31 2f 31 20 62 6c 75 65  |reate X 1/1 blue|
00000080  20 46 61 65 72 69 65 20  63 72 65 61 74 75 72 65  | Faerie creature|
00000090  20 74 6f 6b 65 6e 73 20  77 69 74 68 20 66 6c 79  | tokens with fly|
000000a0  69 6e 67 2e 00 00 00 00  02 00 00 00 0f f1 02 00  |ing.............|
000000b0  06 00 00 00 7b 31 7d 7b  57 7d 02 00 00 00 07 00  |....{1}{W}......|
000000c0  00 00 53 6f 72 63 65 72  79 51 00 00 00 50 75 74  |..SorceryQ...Put|
000000d0  20 74 61 72 67 65 74 20  63 72 65 61 74 75 72 65  | target creature|
000000e0  20 77 69 74 68 20 70 6f  77 65 72 20 34 20 6f 72  | with power 4 or|
000000f0  20 67 72 65 61 74 65 72  20 6f 6e 20 74 68 65 20  | greater on the |
00000100  62 6f 74 74 6f 6d 20 6f  66 20 69 74 73 20 6f 77  |bottom of its ow|
00000110  6e 65 72 27 73 20 6c 69  62 72 61 72 79 2e 00 00  |ner's library...|
00000120  00 00 01 00 00 00 f8 ef  02 00 06 00 00 00 7b 57  |..............{W|
00000130  7d 7b 42 7d 02 00 00 00  18 00 00 00 43 72 65 61  |}{B}........Crea|
00000140  74 75 72 65 20 2d 20 5a  6f 6d 62 69 65 20 4b 6e  |ture - Zombie Kn|
00000150  69 67 68 74 60 00 00 00  57 68 65 6e 65 76 65 72  |ight`...Whenever|
00000160  20 61 6e 6f 74 68 65 72  20 63 72 65 61 74 75 72  | another creatur|
00000170  65 20 65 6e 74 65 72 73  20 74 68 65 20 62 61 74  |e enters the bat|
00000180  74 6c 65 66 69 65 6c 64  20 75 6e 64 65 72 20 79  |tlefield under y|
00000190  6f 75 72 20 63 6f 6e 74  72 6f 6c 2c 20 65 61 63  |our control, eac|
000001a0  68 20 6f 70 70 6f 6e 65  6e 74 20 6c 6f 73 65 73  |h opponent loses|
000001b0  20 31 20 6c 69 66 65 2e  03 00 00 00 32 2f 32 01  | 1 life.....2/2.|
000001c0  00 00 00 83 cc 02 00 06  00 00 00 7b 31 7d 7b 42  |...........{1}{B|
000001d0  7d 02 00 00 00 16 00 00  00 43 72 65 61 74 75 72  |}........Creatur|
000001e0  65 20 2d 20 48 75 6d 61  6e 20 52 6f 67 75 65 71  |e - Human Rogueq|
000001f0  00 00 00 44 65 61 74 68  74 6f 75 63 68 0a 41 66  |...Deathtouch.Af|
00000200  74 65 72 6c 69 66 65 20  31 20 28 57 68 65 6e 20  |terlife 1 (When |
00000210  74 68 69 73 20 63 72 65  61 74 75 72 65 20 64 69  |this creature di|
00000220  65 73 2c 20 63 72 65 61  74 65 20 61 20 31 2f 31  |es, create a 1/1|
00000230  20 77 68 69 74 65 20 61  6e 64 20 62 6c 61 63 6b  | white and black|
00000240  20 53 70 69 72 69 74 20  63 72 65 61 74 75 72 65  | Spirit creature|
00000250  20 74 6f 6b 65 6e 20 77  69 74 68 20 66 6c 79 69  | token with flyi|
00000260  6e 67 2e 29 03 00 00 00  31 2f 32 01 00 00 00     |ng.)....1/2....|
0000026f
note: hexdump does not produce colored output, that was added manually to help you understand the example better. Every (unsigned int) card.id is colored DarkKhaki. Every (uint32_t) strlen(card.cost) is colored DarkSalmon. Every (char[]) card.cost is colored DarkOrchid. Every (unsigned int) card.converted_cost is colored DodgerBlue. Every (uint32_t) strlen(card.type) is colored FireBrick. Every (char[]) card.type is colored Green. Every (uint32_t) strlen(card.text) is colored Gold. Every (char[]) card.text is colored IndianRed. Every (uint32_t) strlen(card.stats) is colored OrangeRed. Every (char[]) card.stats is colored Yellow. Every (enum rarity) (card.rarity) is colored SpringGreen.

$ ./search

>> Eternal Isolation
Eternal Isolation                   {1}{W}
Sorcery                           uncommon
------------------------------------------
Put target creature with power 4 or greater on the bottom of its owner's library.
------------------------------------------
                                          

>> Does not exist
./search: 'Does not exist' not found!
>> Corpse Knight
Corpse Knight                       {W}{B}
Creature - Zombie Knight          uncommon
------------------------------------------
Whenever another creature enters the battlefield under your control, each opponent loses 1 life.
------------------------------------------
                                       2/2

>> q

# Example 2
$ make && ./parser

./parser: an input path is required!

# Example 3
$ make && ./parser does/not/exist.csv

There is nothing printed to stdout, shown below is what is printed to stderr.

./parser: cannot open("does/not/exist.csv"): No such file or directory

# Example 4
$ make && ./search

There is nothing printed to stdout, shown below is what is printed to stderr.

./search: cannot open("index.bin"): No such file or directory
$ touch index.bin && ./search

There is nothing printed to stdout, shown below is what is printed to stderr.

./search: cannot open("cards.bin"): No such file or directory */