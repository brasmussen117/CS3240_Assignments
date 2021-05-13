## Instructions:
For this assignment, create a program called parser that takes the included .csv file and outputs a sorted list of entries to the screen.

1. Parse each line (entry) in the file into a card struct. See the included header file for a definition. Put this into a card.h file and include it in your code file!
2. When encountering a duplicate (a card with the same name as a card already read), you must check the card id field and keep the card with the highest id value!
3. Once the entire dataset is in memory, sort them by card name using your favorite sorting method (hint-hint Qsort). The output width should be 52 characters, the description is not bounded by this.
4. Output the sorted entries to stdout. 

## CSV Pitfalls
There are some things to keep in mind with the included dataset:

There are some duplicate cards (they have the exact same name). Do not add them when parsing.
Some cards are missing the stats field and thus have two consecutive delimeters (,,). The function strtok() does not handle this properly, write your own parser!
Two fields are not used in the output, but should still be parsed and included in the card struct for all entries.
Notice that each field in the .csv that is text begins and ends with a quote (id and converted_cost do not, they are numbers), but that isn't shown in the output. Get rid of them!
Also notice that the card's text field sometimes has two double-quotes. This is the proper way to encode a literal double-quote in a .csv file. When outputting to a file, there should only be one shown!
Finally, any actual new line in the card's text field is shown with the actual \ (backslash) and n characters. You must turn these into a '\n' (newline) character!
When pretty-printing remember that the stats field might be missing, in that case, the row that would should the stat field should be filled with spaces.

## Makefile
Remember, you must write your own makefile, such that when your repository is cloned, one only has to run make in the folder to produce the parser application. Also write a clean target that removes binaries or .o object files, so that we may make your program from scratch easily.

The program must be compiled with -std=gnu11, -Werror, and -Wall

## First Example:
$ make && ./parser short-cards-1.csv

Corpse Knight                                 {W}{B}
Creature - Zombie Knight                    uncommon
----------------------------------------------------
Whenever another creature enters the battlefield under your control, each opponent loses 1 life.
----------------------------------------------------
                                                 2/2

Eternal Isolation                             {1}{W}
Sorcery                                     uncommon
----------------------------------------------------
Put target creature with power 4 or greater on the bottom of its owner's library.
----------------------------------------------------
                                                    

Orzhov Enforcer                               {1}{B}
Creature - Human Rogue                      uncommon
----------------------------------------------------
Deathtouch
Afterlife 1 (When this creature dies, create a 1/1 white and black Spirit creature token with flying.)
----------------------------------------------------
                                                 1/2

Stolen by the Fae                          {X}{U}{U}
Sorcery                                         rare
----------------------------------------------------
Return target creature with converted mana cost X to its owner's hand. You create X 1/1 blue Faerie creature tokens with flying.
----------------------------------------------------
                                                    

$ cat short-cards-1.csv

"id","name","cost","converted_cost","type","text","stats","rarity"
198981,"Stolen by the Fae","{X}{U}{U}",2,"Sorcery","Return target creature with converted mana cost X to its owner's hand. You create X 1/1 blue Faerie creature tokens with flying.",,"rare"
192783,"Eternal Isolation","{1}{W}",2,"Sorcery","Put target creature with power 4 or greater on the bottom of its owner's library.",,"uncommon"
192783,"Eternal Isolation","{1}{W}",2,"Sorcery","Put target creature with power 4 or greater on the bottom of its owner's library.",,"uncommon"
192504,"Corpse Knight","{W}{B}",2,"Creature - Zombie Knight","Whenever another creature enters the battlefield under your control, each opponent loses 1 life.","2/2","uncommon"
183427,"Orzhov Enforcer","{1}{B}",2,"Creature - Human Rogue","Deathtouch\nAfterlife 1 (When this creature dies, create a 1/1 white and black Spirit creature token with flying.)","1/2","uncommon"

## Second Example:
$ make && ./parser short-cards-2.csv

Bomrek of the Thornhand                          {4}
Performance - Poet                          uncommon
----------------------------------------------------
He thinks that the world should be engaged in
perpetual warfare. He has a great deal of respect
for worthy crafts[man]ship. He doesn't really value
merrymaking. He sees working hard as a foolish waste
of time. He does not respect the law. He dislikes
cooperation. He doesn't have any strong feelings
about tradition. He needs alcohol to get through the
working day
----------------------------------------------------
                                                 7/2

Ilral                                      {3}{G}{G}
Engineer - Siege operator                     mythic
----------------------------------------------------
She finds artwork boring. She is offended by leisure
time and leisurely living. She sees cooperation as
very important in life. She sees equal parts of
harmony and discord as part of life. She doesn't see
much value in being stoic. She sees competition as
reasonably important. She thinks that introspection
is valueless and those that waste time in self-
examination are deluded fools. She needs alcohol to
get through the working day
----------------------------------------------------
                                                 5/7

Mafol Rockcrusher                       {X}{2}{U}{U}
Ranger - Trapper                                rare
----------------------------------------------------
She finds friendship burdensome. She is an absolute
believer in the rule of law. She sees guile and
cunning as indirect and somewhat worthless. She
doesn't value eloquence so much. She values hard
work. She doesn't particularly care between war and
peace. She finds romance distasteful. She needs
alcohol to get through the working day
----------------------------------------------------
                                                 4/4

Onol Waranvil                              {T}{4}{U}
Other Jobs - Alchemist                          rare
----------------------------------------------------
She values independence. She holds the view that
commerce is a vile obscenity. She finds friendship
burdensome. She holds crafts[man]ship to be of the
highest ideals and celebrates talented artisans and
their masterworks. She feels that introspection and
all forms of self-examination are the keys to a good
life and worthy of respect. She believes that the
acquisition of power over others is the ideal goal
in life and worthy of the highest respect. She finds
eloquence and artful speech off-putting. She needs
alcohol to get through the working day
----------------------------------------------------
                                                 2/2

$ cat short-cards-2.csv

"id","name","cost","converted_cost","type","text","stats","rarity"
4262949024,"Bomrek of the Thornhand","{4}",4,"Performance - Poet","He thinks that the world should be engaged in\nperpetual warfare. He has a great deal of respect\nfor worthy crafts[man]ship. He doesn't really value\nmerrymaking. He sees working hard as a foolish waste\nof time. He does not respect the law. He dislikes\ncooperation. He doesn't have any strong feelings\nabout tradition. He needs alcohol to get through the\nworking day","7/2","uncommon"
3411778384,"Ilral","{3}{G}{G}",6,"Engineer - Siege operator","She finds artwork boring. She is offended by leisure\ntime and leisurely living. She sees cooperation as\nvery important in life. She sees equal parts of\nharmony and discord as part of life. She doesn't see\nmuch value in being stoic. She sees competition as\nreasonably important. She thinks that introspection\nis valueless and those that waste time in self-\nexamination are deluded fools. She needs alcohol to\nget through the working day","5/7","mythic"
1679258629,"Mafol Rockcrusher","{X}{2}{U}{U}",1,"Ranger - Trapper","She finds friendship burdensome. She is an absolute\nbeliever in the rule of law. She sees guile and\ncunning as indirect and somewhat worthless. She\ndoesn't value eloquence so much. She values hard\nwork. She doesn't particularly care between war and\npeace. She finds romance distasteful. She needs\nalcohol to get through the working day","4/4","rare"
1983949280,"Onol Waranvil","{T}{4}{U}",4,"Other Jobs - Alchemist","She values independence. She holds the view that\ncommerce is a vile obscenity. She finds friendship\nburdensome. She holds crafts[man]ship to be of the\nhighest ideals and celebrates talented artisans and\ntheir masterworks. She feels that introspection and\nall forms of self-examination are the keys to a good\nlife and worthy of respect. She believes that the\nacquisition of power over others is the ideal goal\nin life and worthy of the highest respect. She finds\neloquence and artful speech off-putting. She needs\nalcohol to get through the working day","2/2","rare"

## Third Example 
(in the case that the input file does not exist)
$ make && ./parser does-not-exist.csv 1> /dev/null

(notice this output is from stderr, not stdout, there should be nothing printed on stdout)
Also, the exit-code of the program must be the integer value 1.

./parser: cannot open("does-not-exist.csv"): No such file or directory
$ echo $?
