// InstructionFile.cpp
#ifndef INSTRUCTIONFILE_CPP
#define INSTRUCTIONFILE_CPP
#pragma once

// Compiler Directive
using namespace std;


class InstructionFile{
	// Private variables for InstructionFile
	VirtualDisk* currentDisk;
	std::string fileName;
	ifstream inputFileStream;
	std::queue<queue<string> > userInstructions;
public:
	// Function declarations/prototypes
	InstructionFile(VirtualDisk* disk);
	int setupFile();
	int getUserFileName();
	int openFile();
	void closeFile();
	int processFileContents();
	void executeNextInstruction();
	void executeAllInstructions();
	std::queue<string> split(std::string &inputString, char delimiter=',');
	int doCommand(std::queue<string> &command);
	int makeLowercase(std::string &inputString);
	
	void doAdd(std::queue<string> &command);
	void doRead(std::queue<string> &command);
	void doDelete(std::queue<string> &command);
};


#endif