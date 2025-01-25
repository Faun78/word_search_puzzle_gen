# Word Search puzzle with secret message (Osmisměrka)

Created by Vladimír Sklenár, Ríša Hladík and it was tested by Daniel Culliver for [KSP](https://ksp.mff.cuni.cz/) .
Correspodence Seminar in Programming for Czech and Slovak high school students.

## Task

We wanted to create a simple task for high school students to solve a word search puzzle. If you want to look at
the story for the task you can go to our website [KSP year 2024](https://ksp.mff.cuni.cz/z/ulohy/37/zadani3.html#task-37-Z3-3).

High school students are supposed to create a program that will find all words and print the remaining letters in the grid
as the solution. The problems that we(as orgs) faced were that we needed to generate a different puzzle for each contestant
and all of them are supposed to be generated in max time of 5 seconds. We also wanted to have a secret message
and when you place words randomly in the grid, sometimes you will create a phantom word that will delete the secret message.

## Solution

We created a program that generates a word search puzzle with a secret message. Then the program solves the puzzle
and deletes all the words that create a phantom word. Then these words are replaced with one of the 4 random characters["w","q","x","#"]
(characters were picked purposefully to make sure they will not be in any of the secret messages, they are one of the least common characters in the Czech language).
After these steps the program prints the puzzle and dictionary of words that are supposed to be found in the puzzle. Words can be placed multiple times and can
overlap. The words appear in all 8 directions. The puzzle solver is an implementation of the Aho-Corasick algorithm with some magic that
looks terrible(we wanted to ship the program in time and none of us I accept the blame or you can fix it by pull request I will appreciate it).

## Usage

Firstly you must have a C++ compiler installed on your machine. Then you must
download some dictionary of words that has each word on a new line. I cannot reveal our dictionary
but I am sure you can find one. The dictionary should be placed in dictionary.txt. After that you can create another file in the same directory as the executable
and name it secret.txt. Then write one number on the first line that will represent the number of rows with secret message.
Code will pick one of these secrets randomly and it will integrate it into the puzzle. Then you can run and compile the program:

The program is written in C++ and you can compile it with the following command where you 
replace <dimensions> with the dimensions of the puzzle and <seed> with the seed for the random generator.:

```bash
g++ genrator -o generator.cpp 
./generator <dimensions> <seed> >puzzle.txt
```

The program will print the puzzle to the standard output and you can redirect it to a file.
The program will also print the dictionary of words that are supposed to be found in the puzzle.

## License

Realization of this project is financially supported by the Ministry of Education, Youth and Sports of the Czech Republic.

The content is licensed under Creative Commons [CC-BY-NC-SA 3.0](https://creativecommons.org/licenses/by-nc-sa/3.0/cz/deed.en)
