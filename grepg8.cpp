#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <unistd.h>
#include <regex>
#include <dirent.h>
#include <cstdlib>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <omp.h>
#include <limits.h>

using namespace std;

struct Settings {
	bool recursive;
	bool invert;
	bool verbose;
	bool isFile;
	bool extra;
	int numExtra;
	string file;
	string term;
};

void helpCheck(char *argv[]) {
	if (argv[1] == string("-h") || argv[1] == string("--help") || argv[1] == string("-help")) {
		cout << "\ngrepg8\n" << endl;
		cout << "How to use:\n\n";
		cout << "    grepg8 [-A <#>|-f <file>|-r|-v|-V] <search term>\n" << endl;
		cout << "    Modes:\n";
		cout << "    -A    After Context         grepg8 will grab a number of lines after the line containing the <search term>.\n";
		cout << "    -f    Single File Search    Signals grepg8 to only search the <file> for the <search term>\n";
		cout << "    -r    Recursive Search      Recursively searches through the directory and all sub directories for the given <search term>\n";
		cout << "    -v    Search Inversion      Search for every line that does not include the <search term>.\n";
		cout << "    -V    Enable Verbose        The file path to the file will be printed along with the search result.\n";
		exit(0);
	}
}

void printWords(string str, Settings *instance)
{
    string word;
    regex rgx((*instance).term);
	// cout << (*instance).term << endl;
    stringstream iss(str);
    while (iss >> word) {
        if(regex_match(word.begin(), word.end(), rgx)) {
            cout << "\033[1;34m" << word << "\033[0m ";
        }
        else cout << word << " ";
    }
}

void getSettings(int argc, char *argv[], Settings *instance) {
	queue<string> settings;
	for (int i = 1; i < argc; i++) {
		settings.push(argv[i]);
		// cout << argv[i] << endl;
	}
	
	while (!settings.empty()) {
		string arg = settings.front();
		if (arg == "-r") {
			(*instance).recursive = true;
		} 
		else if (arg == "-v") {
			(*instance).invert = true;
		} 
		else if (arg == "-V") {
			(*instance).verbose = true;
		} 
		else if (arg == "-f") {
			(*instance).isFile = true;
			settings.pop();
			if (!settings.empty()) {
				arg = settings.front();
			} 
			else {
				cout << "ERROR: The path to the file was not given. \"grepg8 -h\" for help." << endl;
				exit(0);
			}
			if (arg.compare(0, 1, "-") == 0) {
				cout << "ERROR: The path to the file was not given. \"grepg8 -h\" for help." << endl;
				exit(0);
			}
			(*instance).file = arg;
		}
		else if (arg == "-A") {
			(*instance).extra = true;
			settings.pop();
			if (!settings.empty()) {
				arg = settings.front();
			} else {
				cout << "ERROR: The number of after context lines was not given. \"grepg8 -h\" for help." << endl;
				exit(0);
			}
			if (arg.compare(0, 1, "-") == 0) {
				cout << "ERROR: The number of after context lines was not given. \"grepg8 -h\" for help." << endl;
				exit(0);
			}
			(*instance).numExtra = stoi(arg);
		} else {
			if (settings.size() > 1) {
				cout << "ERROR: grepg8 was called incorrectly. \"grepg8 -h\" for command syntax." << endl;
				exit(0);
			}
			(*instance).term = arg;
		}
		settings.pop();
	}
	if ((*instance).term == "") {
		cout << "ERROR: Search term not given. \"grepg8 -h\" for help." << endl;
		exit(0);
	}
}

void printLines(queue<string> *filePaths, Settings *instance) {
	while (!(*filePaths).empty()) {
		ifstream file1((*filePaths).front());
		string line1;
		regex rgx((*instance).term);
		unsigned long long count = 0;

		for (unsigned long long i = 0; getline(file1, line1); ++i) {
			count++;
		}
		
		int numThreads = omp_get_max_threads();
		unsigned long long blockSize = count / numThreads + 1;

		#pragma omp parallel for schedule(static)
		for (int i = 0; i < numThreads; ++i) {
			ifstream file2((*filePaths).front());
			string line2;
			string output;
			unsigned long long start = i * blockSize;
			
			for (unsigned long long j = 0; j < min(count, start); ++j) {
				getline(file2, line2);
			}

			for (unsigned long long j = start; j <= min(count, start + blockSize); ++j) {
				output = "";
				if ((*instance).verbose) {
					if (!regex_search(line2.begin(), line2.end(), rgx) && (*instance).invert) {
						output += (*filePaths).front() + ": " + line2 + "\n";
					} 
					else if (regex_search(line2.begin(), line2.end(), rgx) && !(*instance).invert) {
						output += (*filePaths).front() + ": " + line2 + "\n";
						if ((*instance).extra) {
							try {
								for (int k = 0; k < (*instance).numExtra; ++k) {
									getline(file2, line2);
									output += line2 + "\n";
									++j;
									if (regex_search(line2.begin(), line2.end(), rgx)) {
										k = 0;
									}
								}
							} 
							catch (...) {
								cout << "ERROR: Could not grab line because it did not exist.\n";
							}
						}
					}
				} 
				else {
					if (!regex_search(line2.begin(), line2.end(), rgx) && (*instance).invert) {
						output += line2 + "\n";
					} 
					else if (regex_search(line2.begin(), line2.end(), rgx) && !(*instance).invert) {
						output += line2 + "\n";
						if ((*instance).extra) {
							try {
								for (int k = 0; k < (*instance).numExtra; ++k) {
									getline(file2, line2);
									output += line2 + "\n";
									++j;
									if (regex_search(line2.begin(), line2.end(), rgx)) {
										k = 0;
									}
								}
							} 
							catch (...) {
								cout << "ERROR: Could not grab line because it did not exist.\n";
							}
						}
					}
				}
				if ((*instance).extra) {
					if (output != "") {
						printWords(output, instance);
						cout << "------\n";
					}
				} 
				else {
					printWords(output, instance);
				}
				getline(file2, line2);
			}
		}
		(*filePaths).pop();
		cout << endl;
	}
}

void findAll(queue<string> *filePaths, const char *cwd, Settings *instance) {
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(cwd)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			string fileBuff = string(ent -> d_name);
			if (fileBuff != "." && fileBuff != "..") {
				DIR *dir2;
				string fileName = string(cwd) + "/" + fileBuff;
				if ((dir2 = opendir(fileName.c_str())) != NULL) {
					closedir(dir2);
					if ((*instance).recursive) {
						findAll(filePaths, fileName.c_str(), instance);
					}
				} 
				else {
					(*filePaths).push(fileName);
				}
			}
		}
		closedir(dir);
	}
}

char cwd[PATH_MAX];

int main(int argc, char *argv[]) {
	Settings *instance = new Settings;
	queue<string> *filePaths = new queue<string>;

	helpCheck(argv);
	getSettings(argc, argv, instance);
	getcwd(cwd, PATH_MAX);

	if ((*instance).isFile) {
		(*filePaths).push(string(cwd) + "/" + (*instance).file);
	} 
	else {
		findAll(filePaths, cwd, instance);
	}
	
	printLines(filePaths, instance);

	cout << endl;

	delete(filePaths);
	delete(instance);
	return 0;
}
