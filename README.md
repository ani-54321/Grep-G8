# Grep-G8

The [grep](https://en.wikipedia.org/wiki/Grep) command line utility was first introduced in 1974 as a method of searching files and directories using regular expressions. With the penetration of big data into every facet of the computing space datasets have grown exponentially in size. The original single-threaded grep implementation is too slow to effectively parse and search multi-gigabyte files and directories. We introduce perg which is a C++ multi-threaded implementation of grep developed at the National Center fo Supercomputing Applciations.

## Requirements
Need at least either GNU C Compiler (4.9) with OpenMP or Intel C Compiler with OpenMP

Compiler must have C++11 support.

## Installation
### GNU C Compiler
```
g++ -std=c++11 -fopenmp -O3 -o bin/perg src/perg.cpp
```

#### Makefile
```
make
make install
```

### Intel C Compiler
```
icc -std=c++11 -openmp -O3 -o bin/perg src/perg.cpp
```

#### Makefile

```
make icc
make install
```

## Usage

### Command Syntax:
```
perg [-A <#> | -f <file> | -h | -r | -v | -V | -w] <search term>
```

### Modes:
```
-A    After Context         grepg8 will grab a number of lines after the line containing the <search term>
-f    Single File Search    Signals grepg8 to only search the <file> for the <search term>
-r    Recursive Search      Recursively searches through the directory and all sub directories for the given <search term>
-v    Search Inversion      Search for every line that does not include the <search term>
-V    Enable Verbose        The file path to the file will be printed along with the search result

```
