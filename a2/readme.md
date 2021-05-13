# CSV Read and Search

## Author
Brenden Rasmussen

## Description
The purpose of this application is to read a csv file containing data in a known order and format. The application is in two parts: csv reader and binary file searcher. The csv reader takes commandline input filename/path and builds a binary file index and a binary file with the remaining details. The binary search takes text input and searches the index binary file for a match, then takes the returned position data to search the details binary file.

As is the case with C programming, memory is actively managed by the program itself.
