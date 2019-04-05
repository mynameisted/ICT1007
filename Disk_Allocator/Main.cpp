/*
// Compile and run program
cls && g++ Main.cpp VirtualDisk.cpp InstructionFile.cpp -o Main && Main

*/

//Preprocessor directive
#include "Header.h"
#include "VirtualDisk.h"
#include "InstructionFile.h"

// Compiler Directive
using namespace std;


// Declare forward external variable linkage
extern const int MAX_ELEMENTS = 128;


/* Main function definition */
int main(){	
	// Create new instance of VirtualDisk
	VirtualDisk newDisk;	
	// Create new instance of InstructionFile
	InstructionFile newInstructions(&newDisk);
	// Execute all instructions in InstructionFile
	newInstructions.executeAllInstructions();
	
	// End of Main function
	return 0;
}