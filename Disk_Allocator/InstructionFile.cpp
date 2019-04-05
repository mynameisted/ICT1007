#include "Header.h"				// Header file
#include "VirtualDisk.h"
#include "InstructionFile.h"	// InstructionFile file

// Compiler Directive
using namespace std;


/*
 * Contructor for InstructionFile
 *
 */
InstructionFile::InstructionFile(VirtualDisk* disk){
	currentDisk = disk;
	cout << endl << "An instruction file in comma-delimited format is required for this program to execute accordingly." << endl;
	setupFile();
}


/*
 * Initiates setup of Instruction File
 *	This method asks the user to input a file (comma delimited)
 *	then calls the openFile method to open the file. The file
 *	contents will be processed and tokenized into the class 
 *	variable userInstructions.
 *
 * Returns:
 *		'1' when completed successfully
 *		
 */

int InstructionFile::setupFile(){
	// Get user to input a file to be read
	getUserFileName();
	// Open the file
	openFile();
	// Process the contents within the file
	processFileContents();
	// Close the file
	closeFile();
  	return 1;
}



/*
 * Gets user input for file name
 *	This method asks the user to input the file name of
 * 	the instructions. The file name is validated to be
 *	greater than length of 5 characters (x.csv), then
 *	saves the file name to fileName.
 *	
 * Input:
 *		Via cin 
 *
 * Returns:
 *		'1' when completed successfully
 *
 */
int InstructionFile::getUserFileName(){
	// Prompt user to enter a file
	cout << "Please enter the full file name (e.g sample.csv): ";
	bool valid = false;
	while (!valid){
		// Save user input to fileName
		cin >> fileName;
		if (fileName.length() < 5){
			// If fileName length is less than 5 (.txt), getUserFileName() again.
			cout << "Please enter a valid file name which includes the file extension. (e.g. sample.csv)" << endl;
			cout << "Please enter the full file name: ";
		} else {
			// Valid file name, exit method.
			valid = true;
			return 1;
		}
	}
}


/*
 * Open file stream from file
 *	This method uses the inputFileStream to open
 *	the file specified in the variable fileName.
 *	
 * Returns:
 *		'1' if stream is opened successfully
 *		'0' if error is encountered
 *	
 */

int InstructionFile::openFile(){
	// Declare new input file stream
	// Open file
	inputFileStream.open(fileName);
	// Verify if file is opened successfully
	if (inputFileStream.is_open()){
		return 1;
	} else {
		// Print error, exit method
		std::cerr << "Error opening file, please try again." << endl;
		return 0;
	}
}	

/*
 * Process File Contents
 *	This method reads in the file content from the stream
 * 	line by line and calls the split() method for each line. 
 *	The set of tokens are then added to userInstructions.
 *
 * Returns:
 *		'1' when completed successfully
 *
 */

int InstructionFile::processFileContents(){
	std::string newLine;
    std::queue<string> tokenSet;
	// Read each line of the file
	while (std::getline(inputFileStream, newLine)){
		// Strip newline of white spaces
		newLine.erase(std::remove(newLine.begin(), newLine.end(), ' '), newLine.end());
		// Call split method on every line, save returned tokens to tokenSet
		tokenSet = split(newLine);
		// Add the tokenSet into userInstructions
		userInstructions.push(tokenSet);
	}
	return 1;
}


/*
 * Split string by delimiter
 *	Default delimiter of ','
 *
 * Input:
 *		inputString - Text string to be processed
 *		delimiter - Character delimiter to split tokens, default delimiter is comma
 *
 * Returns:
 *		Vector of strings
 */

std::queue<string> InstructionFile::split(std::string &inputString, char delimiter){
	std::queue<std::string> commandTokens;
	std::string token;
	std::istringstream tempTokenStream(inputString);
	int intToken;
	while (std::getline(tempTokenStream, token, delimiter)){
		// If token length is greater than 0 (meaning not blank)
		if (token.length() > 0){
			if(commandTokens.size() >= 1){
				// Check data entries
				// Check if string's ending character is \r
				if (token[token.size()-1] == '\r'){
					// Remove the trailing \r character
					token = token.substr(0, token.size()-1);
					// If the token is not empty after cleaning
					if (token.size()){
						// Convert to int to check
						intToken = std::atoi(token.c_str());
						if(intToken !=0){
							// Valid int
							// Add the token to commandTokens
							commandTokens.push(std::to_string(intToken));							
						} else {
							// Invalid int
							// Clear queue and return
							std::queue<string> empty;
							std::swap(commandTokens, empty);
							return commandTokens;
						}
					}
				} else {
					// Convert to int to check
					intToken = std::atoi(token.c_str());
					if(intToken !=0){
						// Valid int
						// Add the token to commandTokens
						commandTokens.push(std::to_string(intToken));						
					} else {
						// Invalid int
						// Clear queue and return
						std::queue<string> empty;
						std::swap(commandTokens, empty);
						return commandTokens;
					}
				}
			} else {
				// Instruction entries
				// Check if string's ending character is \r
				if (token[token.size()-1] == '\r'){
					// Remove the trailing \r character
					token = token.substr(0, token.size()-1);
					// If the token is not empty after cleaning
					if (token.size()){
						// Add the token to commandTokens
						commandTokens.push(token);						
					}
				} else {
					commandTokens.push(token);
				}
			}

		}
	}
   	// If commandTokens has only 1 element (Delimiter not present, no splitting occured)
	if (commandTokens.size() == 1) {
		// Print warning to console
		cerr << "[WARNING] Delimiting character/File data not found. Program may not work as intended." << endl;
	}
   return commandTokens;
}


/*
 * Close file stream
 *	This method closes the file stream.
 *	
 */
void InstructionFile::closeFile(){
	// If thre is an open file stream
	if (inputFileStream.is_open()){
		// Close stream
		inputFileStream.close();
	}
}	


/*
 * Executes next instruction
 *	This method calls the doCommand method to
 *	parse and execute the command.
 *	
 */
void InstructionFile::executeNextInstruction(){
	if (!userInstructions.empty()){
		// Identify and execute respective instruction method
		doCommand(userInstructions.front());
		// Remove (dequeue) the instruction from the queue
		userInstructions.pop();
	} else {
		// No instructions in the queue, print error
		cerr << "No instructions found." << endl;
	}
}

/*
 * Executes all instructions
 *	This method loops through each set of instructions in
 *	userInstructions and calls the doCommand method to
 *	parse and execute the command.
 *	
 */
void InstructionFile::executeAllInstructions(){
	// Check that the queue is not empty
	if (!userInstructions.empty()){
		// Queue not empty, process every element in the queue
		while (!userInstructions.empty()){
			// Identify and execute respective instruction method
			doCommand(userInstructions.front());
			// Remove (dequeue) the instruction from the queue
			userInstructions.pop();
		}
	} else {
		// No instructions in the queue, print error
		cerr << "No instructions found." << endl;
	}
	// Print disk map after executing 
	currentDisk->printDiskMap();
}



/*
 * Instruction parser
 *	This method reads in the entire line of instruction in the vector
 *	and calls first index (instruction token) with the makeLowercase() method. 
 *	It then matches the instruction with a pre-determined set of commands and
 *	calls the respective function.
 *
 * Input:
 *   instruction - Vector of tokenized/split line of instruction
 *
 * Returns:
 *		'1' when completed successfully
 *		'0' if commmand is unrecognised
 */

int InstructionFile::doCommand(std::queue<string> &instruction) {
    // Make the instruction token lowercase
	makeLowercase(instruction.front());
	/* determine which instruction was given and execute the appropriate function */
	if (instruction.front() == "add"){
		// Remove (dequeue) command from the queue
		instruction.pop();
		// Call the doAdd method with the instruction
		doAdd(instruction);
	} else if (instruction.front() == "read"){
		// Remove (dequeue) command from the queue
		instruction.pop();
		// Call the doRead method with the instruction
		doRead(instruction);
	} else if (instruction.front() == "delete"){
		// Remove (dequeue) command from the queue
		instruction.pop();
		// Call the doDelete method with the instruction
		doDelete(instruction);
	} else {
		// Unrecognised instruction, print error and exit method
		cout << "Error: Unrecognised instruction/invalid file found." << endl << endl;
		return 0;
	}
	return 1;	
}


/*
 * Make string lowercase
 *	Note: This will overwrite the original data as part of normalisation
 *
 * Input:
 *		inputString - Text string to be processed
 *
 * Returns:
  *		'1' when completed successfully
 */

int InstructionFile::makeLowercase(std::string &inputString){
	// For each character in the string
	for (int i=0; i<inputString.length(); i++){
		// Replace the character with the lowercase version
		inputString[i] = std::tolower(inputString[i]);
	}
	return 1;
}


/*
 * Execute add command
 *
 * Input:
 *		Instruction - current set of instruction to be processed
 *
 */
void InstructionFile::doAdd(std::queue<string> &instruction){
	// Convert file name to int
	int fileName = stoi(instruction.front());
	// If file name is not empty and file name is valid and there is data in the instruction
	if (fileName && fileName % 100 == 0 && instruction.size() > 1 && fileName > 0 && fileName < 10000){
		// Remove the file name from the instruction queue
		instruction.pop();
		// Call the addFile method in VirtualDisk
		currentDisk->addFile(fileName,instruction);
	} else {
		// Print error message
		cerr << "Error: Adding of file " << instruction.front() << " failed due to invalid file name/file data." << endl << endl;
	}

}


/*
 * Execute read command
 *
 * Input:
 *		Instruction - current set of instruction to be processed
 *
 */
void InstructionFile::doRead(std::queue<string> &instruction){
	// Convert file name to int
	int fileName = stoi(instruction.front());
	// If file name is not empty
	if (fileName){
		// Call the readFile method in VirtualDisk
		currentDisk->readFile(fileName);	
	} else {
		// Print error message
		cerr << "Error: Reading of file " << instruction.front() << " failed due to invalid file name." << endl;
	}
}


/*
 * Execute delete command
 *
 * Input:
 *		Instruction - current set of instruction to be processed
 *
 */
void InstructionFile::doDelete(std::queue<string> &instruction){
	// Convert file name to int
	int fileName = stoi(instruction.front());
	// If file name is not empty and file name is valid
	if (fileName && fileName % 100 == 0){
		// Call deleteFile method in VirtualDisk
		currentDisk->deleteFile(fileName);	
	} else {
		// Print error message
		cerr << "Error: Deletion of file " << instruction.front() << " failed due to invalid file name." << endl;
	}
}
