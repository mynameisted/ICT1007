#include "Header.h"
#include "VirtualDisk.h"		// Header file

// Compiler Directive
using namespace std;

/*
 * Contructor for VirtualDisk
 *
 */
VirtualDisk::VirtualDisk() {
	cout << endl;
	cout << "======================================================================" << endl;
	cout << "   _____  _     _               _ _                 _             " << endl;
	cout << "  |  __ \\(_)   | |        /\\   | | |               | |            " << endl;
	cout << "  | |  | |_ ___| | __    /  \\  | | | ___   ___ __ _| |_ ___  _ __ " << endl;
	cout << "  | |  | | / __| |/ /   / /\\ \\ | | |/ _ \\ / __/ _\\` | __/ _ \\| '__|" << endl;
	cout << "  | |__| | \\__ \\   <   / ____ \\| | | (_) | (_| (_| | || (_) | |   " << endl;
	cout << "  |_____/|_|___/_|\\_\\ /_/    \\_\\_|_|\\___/ \\___\\__,_|\\__\\___/|_|   " << endl;
	cout << "                                                                  " << endl;
	cout << "======================================================================" << endl;
	cout << endl;
	cout << "Welcome to your virtual disk. You have " << MAX_ELEMENTS << " entries available for use." << endl;
	// Call methods to set up the virtual disk
	setupDisk();
	// Print VCB details
	printVCB();
	// Print virtual disk map
	printDiskMap();
}

VirtualDisk::~VirtualDisk(){
	// Destructor, free explicitly allocated memory 
	delete DiskVCB;
	delete[] DiskDir;
	delete[] DiskData;
}


/*
 * Check directory structure for inode
 *	This method goes into the virtual disk's directory structure
 *	and returns a pointer to a matching file name. If no value is passed
 *	for the file name, it will search for an empty empty in the
 *	directory structure and return it. If no entry is found, return null.
 * Output:
 *		pointer to the entry in the directory structure
 *		Returns NULL if no entry is found
 */
iNode* VirtualDisk::checkINode(int file=-1){
	// Initialise a temp pointer to our directory structure
	iNode* tempDirPtr = DiskDir;
	// Loop through our directory structure
	for (int i=0; i < DiskVCB->blockSize-1; i++){
		// If there is a match to the passed file name
		if (DiskDir[i].FileIdentifier == file){
			// Return a pointer to the entry
			return tempDirPtr;	
		} else {
			// If not a match, move pointer to next entry
			tempDirPtr++;
		}
	}
	// If no entry found, return null
	return NULL;
}




/*
 * Update inode data in directory structure
 *	This method updates the passed inode pointer's values depending
 *	on the allocation method
 *
 *		
 */
int VirtualDisk::updateINode(iNode* iNodeEntry, int fileName, int startValue, int endValue=-1){
	// Update the file identifier to the passed file name
	iNodeEntry->FileIdentifier = fileName;
	if (allocationMethod == 1){
		// Allocation method 1 - Contiguous
		// Update start block and length
		iNodeEntry->StartBlock = startValue;
		iNodeEntry->Length = endValue;
	} else if (allocationMethod == 2){
		// Allocation method 2 - Linked
		// Update start block and last block
		iNodeEntry->StartBlock = startValue;
		iNodeEntry->LastBlock = endValue;
	} else if (allocationMethod == 3){
		// Allocation method 3 - Indexed
		// Update index block
		iNodeEntry->Index = startValue;
	} else if (allocationMethod == 4){
		// Allocation method 4 - Contiguous Indexed
		// Update index block
		iNodeEntry->Index = startValue;
	}
	return 1;
}




/*
 * Update free space information (bit map + VCB num of free blocks)
 *	This method updates the free block information in the VCB.
 *	The bit map will be updated with the passed value, and the
 *	number of free blocks in the VCB will be changed accordingly.
 *
 */
int VirtualDisk::updateFreeSpace(int startValue, int valueLen, int newValue=0){
	// Check that the existing value is not the same as the passed value
	if (DiskVCB->FreeBlockBitMap[startValue] != newValue){
		// Loop for the passed length of the blocks to update
		for (int i=0;i<valueLen;i++){
			// Update the blt map value to the new value
			DiskVCB->FreeBlockBitMap[startValue+i] = newValue;
		}
		if (newValue == 0){
			// Marking blocks as used, decrease the number of free blocks in VCB
			DiskVCB->numFreeBlock -= valueLen;
		} else if (newValue == 1){
			// Marking blocks as free, increase the number of free blocks in VCB
			DiskVCB->numFreeBlock += valueLen;
		}		
		return 1;
	} else {
		// No change to the value of the free space
		return 0;
	}
}




/*
 * Requests for free blocks
 *	This method checks the free block bit map for free blocks 
 *  of a certain length, and returns the starting block if
 *  available.
 *
 * Returns:
 *		Allocated block number when completed successfully
 *		'-1' if no space available.
 */
int VirtualDisk::requestBlocks(int numBlocks){
	bool validBlocks = true;
	// Check if requested block exceeds total number of free blocks
	if (numBlocks <= DiskVCB->numFreeBlock){
		// Check if number of blocks requested is greater than 1 (contiguous)
		if (numBlocks > 1) {
			// Loop through the bit map
			for (int i=0; i < DiskVCB->FreeBlockBitMap.size();i++){
				// If a free block is found
				if (DiskVCB->FreeBlockBitMap[i] == 1){
					// Loop forward the number of blocks requested
					for (int j=1; j < numBlocks; j++){
						// If any of the blocks in the lookahead is used
						if (DiskVCB->FreeBlockBitMap[i+j] == 0){
							validBlocks = false;
							// Skip the current blocks that have already been searched
							i += j;
							break;
						}
					}
					// If there is a contiguous block found
					if (validBlocks){
						// Return the starting block index
						return i;
					} else {
						validBlocks = true;
					}
				}
			}
		} else {
			// User requested for a single block
			for (int i=0; i < DiskVCB->FreeBlockBitMap.size();i++){
				if (DiskVCB->FreeBlockBitMap[i] == 1){
					// If a free block is found, return the index
					return i;
				}
			}
		}
		// If no free blocks found, return -1
		return -1;
	} else {
		// If requested number of blocks exceeds total free blocks, return -1
		return -1;
	}
}



/*
 * Set up current disk
 *	This method intialises a new VCB for the disk, and
 *  calls all necessary methods to configure the 
 *  parameters of the virtual disk. The methods for
 *	setting up disk data and disk directories is called.
 *	The method to update free space information is also called.
 *
 * Returns:
 *		'1' when completed successfully
 *		
 */
int VirtualDisk::setupDisk(){
	// Initialise new VCB structure
	DiskVCB = new VCB;
	// Get user to input file allocation method
	VirtualDisk::setAllocationMethod();
	// Get user to input block size
	VirtualDisk::setBlockSize();
	// Initialise disk data based on block size and allocation method
	setupDiskData();
	// Set up directory structure
	setupDiskDir();
	return 1;
}



/*
 * Set up current disk data
 *	This method initialises the structures for
 *	storing data in our disk, setting the entry index,
 *	the block index based on the block size, and the
 *	default data value to -1 (empty).
 *
 * Returns:
 *		'1' when completed successfully
 *		
 */
int VirtualDisk::setupDiskData(){
	// Initialise new Data structure array (First block reserved for superblock)
	DiskData = new Data[MAX_ELEMENTS-DiskVCB->blockSize];
	for (int i=DiskVCB->blockSize; i < MAX_ELEMENTS; i++){
		// Write the index of the data entry
		DiskData[i-DiskVCB->blockSize].index = i;
		// Write the block number of the data entry
		DiskData[i-DiskVCB->blockSize].block = nearbyint(i/DiskVCB->blockSize);
		// Write the data of the data entry as -1 (unused)
		DiskData[i-DiskVCB->blockSize].data = -1;
		// If allocation method is Contiguous Indexed
		if (allocationMethod == 4){	
			// Write the length of the data entry as -1 (unused)
			DiskData[i-DiskVCB->blockSize].length = -1;
		}
	}
	return 1;
}




/*
 * Set up current disk directory
 *	This method initialises the directory structure for
 *	the virtual disk.
 *
 * Returns:
 *		'1' when completed successfully
 *		
 */
int VirtualDisk::setupDiskDir(){
	// Initialise new iNode structure array (First entry reserved for VCB)
	DiskDir = new iNode[DiskVCB->blockSize-1];
	for (int i=0; i < DiskVCB->blockSize-1; i++){
		// Write the file identifier of the inode to -1 (unused)
		DiskDir[i].FileIdentifier = -1;
		// Switch case to initialise respective iNODE variables
		switch (allocationMethod){				
			case (1): {
				// Contiguous - Initialise and set StartBlock and Length to -1 (unused)
				DiskDir[i].StartBlock = -1;
				DiskDir[i].Length = -1;		
				break;
			}
			case (2): {
				// Linked - Initialise and set StartBlock and LastBlock to -1 (unused)
				DiskDir[i].StartBlock = -1;
				DiskDir[i].LastBlock = -1;
				break;
			}
			case (3): {
				// Indexed - Initialise and set Index to -1 (unused)
				DiskDir[i].Index = -1;
				break;
			}
			case (4): {
				// Contiguous Indexed - Initialise and set Index to -1 (unused)
				DiskDir[i].Index = -1;
				break;
			}
		}
	}
	return 1;
}




/*
 * Sets user input for block size
 *	This method asks the user to input a block size for
 * 	the virtual disk. blockSize is intepreted to be the number of
 * 	entries in a block, and validated to be an integer that is
 *	not less than 1 (if blockSize is 1, no files can be stored), and
 *	not more than half of the available entries (minimum of 2 blocks)
 *
 * Input:
 *		Via cin 
 *
 * Returns:
 *		'1' when completed successfully
 *
 */
int VirtualDisk::setBlockSize(){
	int tempBlockSize;
	cout << endl << "Please enter the size of each block: ";
	bool valid = false;
	while (!valid){
		// Get user input and save it to a temp variable
		cin >> tempBlockSize;
		// Validate that user input is a number
		while (!cin.good()) {
			// User input invalid, reset input stream
		    cin.clear();
		    cin.ignore(INT_MAX,'\n');
		    // Get user input again
		    cout << "Please enter a valid block size: ";
		    cin >> tempBlockSize;
		}
		// Round tempBlockSize off to nearest int
		tempBlockSize = nearbyint(tempBlockSize);
		// If tempBlockSize is less than 1, getUserBlockSize() again.
		if (tempBlockSize <= 1 || tempBlockSize > MAX_ELEMENTS / 2){
			cout << "Your specified block size of " << tempBlockSize << " has to be greater than 1 and less than " << MAX_ELEMENTS/2 << endl;
			cout << "Please select another block size: ";
		} else if (MAX_ELEMENTS % tempBlockSize != 0){
		// Check for valid block size that fits into our total entries, else getUserBlockSize() again.
			cout << "Your specified block size of " << tempBlockSize << " will result in " << MAX_ELEMENTS % tempBlockSize << " unusable entries." << endl;
			cout << "Please select another block size: ";
		} else {
			valid = true;
			// Save block size to VCB
			DiskVCB->blockSize = tempBlockSize;
			// Calculate and save total number of blocks to VCB
			DiskVCB->totalBlockNum = MAX_ELEMENTS / tempBlockSize;
			// Calculate and save total number of free blocks to VCB (first block used as superblock)
			DiskVCB->numFreeBlock = MAX_ELEMENTS / tempBlockSize - 1;
			// Build free space bit map
			buildFSBitMap();
			return 1;
		}
	}
}




/*
 * Sets user input for disk allocation method
 *	This method gives the user a list of disk allocation methods to 
 * 	choose from (Contiguous, Linked, Indexed, Contiguous Indexed). The user's 
 *	input is validated to be an integer, and must be one of the available
 *	options provided (either 1, 2, 3 or 4).
 *
 * Input:
 *		Via cin 
 *
 * Returns:
 *		'1' when completed successfully
 *
 */
int VirtualDisk::setAllocationMethod(){
	int userInput;
	cout << "Please choose a disk block allocation method" << endl;
	cout << "\t1. Contiguous Allocation" << endl;
	cout << "\t2. Linked Allocation" << endl;
	cout << "\t3. Indexed Allocation" << endl;
	cout << "\t4. Contiguous Indexed Allocation" << endl;
	cout << endl << "Allocation Method (e.g. 1): ";
	bool valid = false;
	// While input is not valid
	while (!valid){
		cin >> userInput;
		// If input is not of type int
		while (!cin.good()){
			// flush the stream and prompt user to try again
			cin.clear();
		    cin.ignore(INT_MAX,'\n');
	    	cout << "Please enter a valid option (e.g. 1): ";
		    cin >> userInput;
		}
		// Round off value to nearest integer
		userInput = nearbyint(userInput);
		// If rounded number is wihin the available options
		if (userInput >=1 && userInput <= 4){
			// Set valid to true to exit the while loop
			valid = true;
			// Save input value to class variable
			allocationMethod = userInput;
			return 1;
		} else {
			// User entered invalid input type
			cout << "Invalid input. Please enter a valid option from 1 to 4: ";
		}
	}
}





/*
 * Builds the free space bit map in the VCB
 *	This method goes into the virtual disk's VCB to build the
 *	free block bit map. The first block is reserved for superblock
 *	and thus marked as used. The rest of the blocks are deemed as free
 *	as this function is called on first initialisation of the disk.
 *		
 */
int VirtualDisk::buildFSBitMap(){
	// Mark first bit as used (superblock)
	DiskVCB->FreeBlockBitMap.push_back(0);
	for (int i=0;i < DiskVCB->totalBlockNum-1;i++){
		// Mark the remaining bits as unused
		DiskVCB->FreeBlockBitMap.push_back(1);
	}
}


/*
 * Prints Volume Control Block (current disk configuration)
 *	This method prints the current virtual disk configurations 
 *
 * Output:
 *		Total entries available for the disk
 *		Block Size (Number of entries per block)
 *		Total number of blocks
 *		Number of free blocks
 *		Free block bit map
 *		Allocation Method
 *		
 */
void VirtualDisk::printVCB(){
	cout << endl << "Your virtual disk has been configured successfully." << endl;
	cout << endl << "Disk Properties" << endl;
	cout << "\t> Total entries available: " << MAX_ELEMENTS << endl;
	cout << "\t> Block Size: " << DiskVCB->blockSize << endl;
	cout << "\t> Total number of blocks: " << DiskVCB->totalBlockNum << endl;
	cout << "\t> Number of free blocks: " << DiskVCB->numFreeBlock << endl;
	//cout << "\t> Free block bit map: " << DiskVCB->FreeBlockBitMap << endl;
	cout << "\t> Allocation Method: " << allocationMethod << " => ";
	// Switch case to translate DiskVCB->s allocationMethod int to the respective allocation method name
	switch (allocationMethod){				
		case (1): {
			cout << "Contiguous allocation" << endl;
			break;
		}
		case (2): {
			cout << "Linked allocation" << endl;
			break;
		}
		case (3): {
			cout << "Indexed allocation" << endl;
			break;
		}
		case (4): {
			cout << "Contiguous Indexed allocation" << endl;
			break;
		}
	}
	cout << endl;

}




/*
 * Prints the disk map to console
 *	This method goes through the virtual disk to print
 *	the data in the disk, then calls the method to print
 *	the free block bit map.
 *
 * Output:
 *		Block index
 *		Entry index
 *		Entry data
 *		
 */
void VirtualDisk::printDiskMap(){
	iNode* tempDirPtr = DiskDir;
	cout << endl;			
	// Print formatting of headers
	cout << setw(printDiskMapWidth) << "Block" << setw(printDiskMapWidth) << "Index" << setw(printDiskMapWidth) << "Data" << endl;
	// Loop for each entry in our disk
	for (int i=0; i < MAX_ELEMENTS; i++){
		// Print block divider for visual identification
		if (i % DiskVCB->blockSize == 0){
			cout << "=======================================" << endl;
		} else {
			// Print entry divider for visual identification
			cout << "---------------------------------------" << endl;		
		}

		// Print Superblock (VCB + Directory Structure)
		if (i < DiskVCB->blockSize){
			cout << setw(printDiskMapWidth) << nearbyint(i/DiskVCB->blockSize) << setw(printDiskMapWidth) << i << setw(printDiskMapWidth);
			if (i == 0){
				// VCB
				cout << DiskVCB->totalBlockNum << "," << DiskVCB->numFreeBlock << "," << DiskVCB->blockSize << ",";
				cout << "[";
				for (int j=0; j < DiskVCB->FreeBlockBitMap.size();j++){
					cout << DiskVCB->FreeBlockBitMap[j];
				}
				cout << "]" << endl;
			} else {
				// Directory Structure (inodes)
				// Switch case to translate allocationMethod int to the respective inode format
				switch (allocationMethod){
					case (1): {
						// Allocation method 1 - Contiguous
						if (tempDirPtr->FileIdentifier == -1){
							// File identifier is -1 (unused), print '-'' instead
							cout << "-" << endl;	
						} else {
							// Print file identifier, start block and length
							cout << tempDirPtr->FileIdentifier << "," << tempDirPtr->StartBlock << "," << tempDirPtr->Length << endl;
						}
						tempDirPtr++;
						break;
					}
					case (2): {
						// Allocation method 2 - Linked
						if (tempDirPtr->FileIdentifier == -1){
							// File identifier is -1 (unused), print '-'' instead
							cout << "-" << endl;	
						} else {
							// Print file identifier, start block and last block
							cout << tempDirPtr->FileIdentifier << "," << tempDirPtr->StartBlock << "," << tempDirPtr->LastBlock << endl;
						}
						tempDirPtr++;
						break;
					}
					case (3): {
						// Allocation method 3 - Indexed
						if (tempDirPtr->FileIdentifier == -1){
							// File identifier is -1 (unused), print '-'' instead
							cout << "-" << endl;	
						} else {
							// Print file identifier and index block
							cout << tempDirPtr->FileIdentifier << "," << tempDirPtr->Index << endl;
						}
						tempDirPtr++;
						break;
					}
					case (4): {
						// Allocation method 4 - Contiguous Indexed
						if (tempDirPtr->FileIdentifier == -1){
							// File identifier is -1 (unused), print '-'' instead
							cout << "-" << endl;	
						} else {
							// Print file identifier and index block
							cout << tempDirPtr->FileIdentifier << "," << tempDirPtr->Index << endl;
						}
						// Move directory structure pointer to the next entry
						tempDirPtr++;
						break;
					}
				}					
			}
		} else{
			// Print Disk Data
			if (allocationMethod == 4){
				// Allocation method 4 - Contiguous Index
				// Set formatting and print block number and index
				cout << setw(printDiskMapWidth) << DiskData[i-DiskVCB->blockSize].block << setw(printDiskMapWidth) << DiskData[i-DiskVCB->blockSize].index << setw(printDiskMapWidth);
				// If data entry is not -1 (used)
				if (DiskData[i-DiskVCB->blockSize].data != -1){
					// If data entry length is specified (used as index block)
					if (DiskData[i-DiskVCB->blockSize].length != -1){
						// Print file data and length
						cout << DiskData[i-DiskVCB->blockSize].data << "," << DiskData[i-DiskVCB->blockSize].length <<  endl;
					} else {
						// Print file data
						cout << DiskData[i-DiskVCB->blockSize].data <<  endl;
					}
				} else {
					// Print empty placeholder
					cout << "-" << endl;
				}			
			} else {
				// Allocation methods 1,2,3 - Contiguous, Linked, Indexed
				// Set formatting and print block number and index
				cout << setw(printDiskMapWidth) << DiskData[i-DiskVCB->blockSize].block << setw(printDiskMapWidth) << DiskData[i-DiskVCB->blockSize].index << setw(printDiskMapWidth);
				// If data entry is not -1 (used)
				if (DiskData[i-DiskVCB->blockSize].data != -1){
					// Print file data
					cout << DiskData[i-DiskVCB->blockSize].data<< endl;
				} else {
					// Print empty placeholder
					cout << "-" << endl;
				}			
			}
		}
	}
	// Print ending divider
	cout << "=======================================" << endl;
	// Call method to print the free space bit map of our disk.
	printFreeSpaceBitMap();
}




/*
 * Prints the free space bit map to console
 *	This method goes into the virtual disk's VCB to access the
 *	free block bit map, then prints out the bit map to console.
 *
 * Output:
 *		Printout of free space bit map array with:
 *		'0' for a used block
 * 		'1' for a free block
 *		
 */
void VirtualDisk::printFreeSpaceBitMap(){
	// Print heading + number of free space / total free space
	cout << endl << "Free space bit map ("<< DiskVCB->numFreeBlock << "/" << DiskVCB->FreeBlockBitMap.size() << ")" << endl;
	cout << "[";
	for (int j=0; j < DiskVCB->FreeBlockBitMap.size();j++){
		// Print each bit of the bit map
		cout << DiskVCB->FreeBlockBitMap[j];
	}
	cout << "]" << endl << endl;
}





/*
 * Adds file data to virtual disk
 *	This method checks if the requested file to be added exists in the file
 *  directory. If yes, the file will not be added. If the file does not exists,
 *	blocks will be requested to store the file on the virtual disk, and respectively
 *	saved to disk using the respective allocated methods.
 *
 * Returns:
 *		'1' when completed successfully
 *		'0' if unsuccessful
 *		
 */
int VirtualDisk::addFile(int fileName, queue<string> fileContents){
	int allocatedBlockStartingAddr = 0;
	int allocatedBlock = 0;
	int dataCount = fileContents.size();
	int accessTime = 0;
	// Check if file name exists in the directory structure
	iNode* tempDirPtr = checkINode(fileName);
	// Increment access time (Accessed memory)
	accessTime++;
	if (tempDirPtr){
		// Existing file name found in directory structure
		cout << "Adding file " << fileName << "." << endl;		
		cout << "Error: File " << fileName << " already exists in the virtual disk." << endl << endl;
		return 0;
	} else {
		// File does not exist in directory structure, request for new iNode entry in directory structure
		tempDirPtr = checkINode();
		// Increment access time (Accessed memory)
		accessTime++;
	}
	if (tempDirPtr){
		// iNode entry obtained successfully
		switch (allocationMethod){				
			case (1): {
				// Allocation method 1 - Contiguous
				// Calculate the number of blocks needed to store the file
				int blocksNeeded = ceil(fileContents.size()*1.00 / DiskVCB->blockSize);
				// Check if there is enough free blocks to support the file
				if (ceil(fileContents.size()*1.00 / DiskVCB->blockSize) > DiskVCB->numFreeBlock){
					// Number of blocks exceeds available number of free blocks on the virtual disk
					cout << "Adding file " << fileName << "." << endl;		
					cout << "Error: File "<< fileName <<"'s size exceeds available space on the virtual disk." << endl << endl;
					return 0;
				}
				// Request for contiguous chunk of blocks
				allocatedBlock = requestBlocks(blocksNeeded);
				// Check if allocation was successful
				if (allocatedBlock != -1){
					// Contiguous block available
					cout << "Adding file " << fileName << " and found free block starting at " << allocatedBlock << endl;
					// Calculate the position of the block in disk data
					allocatedBlockStartingAddr = (allocatedBlock*DiskVCB->blockSize) - DiskVCB->blockSize;
					cout << "Added file " << fileName << " at ";
					// Loop through every entry of the file
					for(int i=0;i < dataCount;i++){
						if (i % DiskVCB->blockSize == 0){
							// Printing of allocation block number
							cout << "B" << DiskData[allocatedBlockStartingAddr+i].block << "(";
							cout << fileContents.front();
							if(fileContents.size() > 1){
								cout << ",";
							} else {
								cout << ") ";
							}
						} else {
							// Printing of file content
							cout << fileContents.front();
							if(fileContents.size() > 1 && i % DiskVCB->blockSize != DiskVCB->blockSize-1){
								cout << ",";
							} else {
								cout << ") ";
							}
						}
						// Write file entry to disk data
						DiskData[allocatedBlockStartingAddr+i].data = stoi(fileContents.front());
						// Increment access time (Accessed memory)
						accessTime++;
						// Delete file entry from the queue
						fileContents.pop();
					}
					// Update the file's inode in the directory structure
					updateINode(tempDirPtr, fileName, allocatedBlock, blocksNeeded);
					// Increment access time (Accessed memory)
					accessTime++;
					// Update the free space bit map and number of free blocks
					updateFreeSpace(allocatedBlock, blocksNeeded);					
					// Increment access time (Accessed memory)
					accessTime++;
				} else {
					// No contiguous block available
					cout << "No available space in disk found." << endl;
					return 0;
				}
				cout << endl;
				break;
			}
			case (2): {
				// Allocation Method 2 - Linked
				// Calculate the number of blocks needed to store the file and if there is enough free blocks to support the file
				if (ceil(fileContents.size()*1.00 / (DiskVCB->blockSize-1)) > DiskVCB->numFreeBlock){
					// Number of blocks exceeds available number of free blocks on the virtual disk
					cout << "Adding file " << fileName << "." << endl;
					cout << "Error: File "<< fileName <<"'s size exceeds available space on the virtual disk." << endl;
					return 0;
				}
				int firstAllocatedBlock;
				for(int i=0; i < dataCount; i++){
					if (fileContents.size() != 0){
						if (fileContents.size() == dataCount){
							// Request 1 block as head of linked list
							allocatedBlock = requestBlocks(1);
							// Save the head block
							firstAllocatedBlock = allocatedBlock;
							cout << "Adding file " << fileName << " and found free block starting at " << firstAllocatedBlock << endl;
							cout << "Added file " << fileName << " at ";		
							if (allocatedBlock != -1){
								// If allocation successful, update free space bit map
								updateFreeSpace(allocatedBlock, 1);
								// Increment access time (Accessed memory)
								accessTime++;
								// Calculate the position of the block in the disk data
								allocatedBlockStartingAddr = (allocatedBlock*DiskVCB->blockSize) - DiskVCB->blockSize;
								cout << "B" << allocatedBlock << "(";
							} else {
								// No blocks allocated
								cout << "No available space in disk found." << endl;
								return 0;
							}
						}
						if (allocatedBlock != -1){
							// Print the file entry
							cout << fileContents.front();
							if(fileContents.size() > 1 && i % (DiskVCB->blockSize-1) != DiskVCB->blockSize-2){
								cout << ",";
							} else {
								cout << ") ";
							}
							// Write file entry to disk data
							DiskData[allocatedBlockStartingAddr+(i % (DiskVCB->blockSize-1))].data = stoi(fileContents.front());
							// Increment access time (Accessed memory)
							accessTime++;
							// Delete file entry from the queue
							fileContents.pop();
						} else {
							// No block available
							cout << "No available space in disk found." << endl;
							return 0;
						}		
						if(i % (DiskVCB->blockSize-1) == DiskVCB->blockSize-2 && fileContents.size() != 0){
							// Before reaching the last entry of the block, request for the next block to set the next block pointer
							allocatedBlock = requestBlocks(1);
							if (allocatedBlock != -1){
								// Write the next block pointer to disk data
								DiskData[allocatedBlockStartingAddr+(i % (DiskVCB->blockSize-1))+1].data = allocatedBlock;
								// Increment access time (Accessed memory)
								accessTime++;
								// Update free space bit map
								updateFreeSpace(allocatedBlock, 1);
								// Calculate offset of the next block in the disk data
								allocatedBlockStartingAddr = (allocatedBlock*DiskVCB->blockSize) - DiskVCB->blockSize;
								cout << "B" << allocatedBlock << "(";
							}
						}
					}
				}
				// Update the file's inode in the directory structure
				updateINode(tempDirPtr, fileName, firstAllocatedBlock, allocatedBlock);
				// Increment access time (Accessed memory)
				accessTime++;
				cout << endl;
				break;
			}
			case (3): {
				// Allocation method 3 - Indexed
				// Calculate the number of blocks needed to store the file and if there is enough free blocks to support the file
				if ((ceil(fileContents.size() > DiskVCB->blockSize*DiskVCB->blockSize)) || ((fileContents.size()/ DiskVCB->blockSize)+1) > DiskVCB->numFreeBlock){
					cout << "Adding file " << fileName << "." << endl;		
					cout << "Error: File "<< fileName <<"'s size exceeds available space on the virtual disk." << endl << endl;
					return 0;
				}
				int indexBlock = 0;
				if(dataCount == fileContents.size()){
					// Request for indx block
					indexBlock = requestBlocks(1);
					// Increment access time (Accessed memory)
					accessTime++;
					// Update free space bit map
					updateFreeSpace(indexBlock, 1);
				}
				int firstAllocatedBlock = 0;
				// Loop for every file entry
				for(int i=0; i < dataCount; i++){
					if (fileContents.size() != 0){
						// For every new block of data
						if(i % DiskVCB->blockSize == 0){
							// Request for one block
							allocatedBlock = requestBlocks(1);
							// Increment access time (Accessed memory)
							accessTime++;
							if (allocatedBlock != -1){
								// Calculate position of current block in disk data
								allocatedBlockStartingAddr = (indexBlock*DiskVCB->blockSize) - DiskVCB->blockSize;
								// Save data block to index block
								DiskData[allocatedBlockStartingAddr+(i / DiskVCB->blockSize)].data = allocatedBlock;
								// Increment access time (Accessed memory)
								accessTime++;
								// Update free block bit map
								updateFreeSpace(allocatedBlock, 1);
								if (dataCount == fileContents.size()){
									// Save first allocated block
									firstAllocatedBlock = allocatedBlock;
									cout << "Adding file " << fileName << " and found free block starting at " << firstAllocatedBlock << endl;
									cout << "Added file " << fileName << " at ";		
								}
								// Calculate position of allocated block in disk data
								allocatedBlockStartingAddr = (allocatedBlock*DiskVCB->blockSize) - DiskVCB->blockSize;
								cout << "B" << allocatedBlock << "(";
							} else {
								cout << "No available space in disk found." << endl;
								return 0;
							}
						}
						if (allocatedBlock != -1){
							// Print file entry
							cout << fileContents.front();
							if(fileContents.size() > 1 && i % DiskVCB->blockSize != DiskVCB->blockSize-1){
								cout << ",";
							} else {
								cout << ") ";
							}
							// Write file entry to disk data
							DiskData[allocatedBlockStartingAddr+(i % DiskVCB->blockSize)].data = stoi(fileContents.front());
							// Increment access time (Accessed memory)
							accessTime++;
							// Delete file entry from the queue
							fileContents.pop();
						} else {
							cout << "No available space in disk found." << endl;
							return 0;
						}
					}
				}
				cout << endl;
				// Update the file's inode in the directory structure
				updateINode(tempDirPtr, fileName, indexBlock);
				// Increment access time (Accessed memory)
				accessTime++;
				break;
			}
			case (4): {
				// Allocation method 4 - Contiguous Indexed
				// Calculate total number of blocks needed
				int blocksNeeded = ceil(fileContents.size()*1.00 / DiskVCB->blockSize);
				// Check if there is enough free blocks to support the file (total blocks needed + 1 index block)
				if(blocksNeeded+1 > DiskVCB->numFreeBlock){
					cout << "Adding file " << fileName << "." << endl;		
					cout << "Error: File "<< fileName <<"'s size exceeds available space on the virtual disk." << endl << endl;
					return 0;
				} else {
					// Request index block
					int indexBlock = requestBlocks(1);
					// Increment access time (Accessed memory)
					accessTime++;
					int entriesUsed = 0;
					std::map<int, int> blocksToUse;
					// Update free space bit map
					updateFreeSpace(indexBlock, 1);
					int tempBlockNum = blocksNeeded;
					// Preprocess blocks needed for the file
					while (tempBlockNum > 0 && blocksNeeded > 0){
						// Get the chunks of blocks necessary for the file
						if(entriesUsed == DiskVCB->blockSize){
							// File entries exceeded index block capacity
							cout << "Error: File "<< fileName <<"'s size exceeds available space on the virtual disk." << endl << endl;
							// Release reserved blocks if allocation failed - Index block
							updateFreeSpace(indexBlock,1,1);
							int tempBlock, tempLength;
							while(blocksToUse.empty() == false){
								// Get block index
								tempBlock = blocksToUse.begin()->first;
								// Get block length
								tempLength = blocksToUse.begin()->second;
								// Release reserved blocks - File data blocks
								updateFreeSpace(tempBlock, tempLength, 1);
								// Delete the block data from our map
								blocksToUse.erase(blocksToUse.begin());
							}
							return 0;
						}
						// Request a contiguous chunk of blocks
						allocatedBlock = requestBlocks(tempBlockNum);
						// Increment access time (Accessed memory)
						accessTime++;
						if(allocatedBlock == -1){
							// If no available chunk of blocks of the size, decrement the number
							tempBlockNum--;
						} else {
							// Update free space bit map with allocated chunk of blocks
							updateFreeSpace(allocatedBlock, tempBlockNum);
							// Insert the allocated block and length of the blocks into the map
							blocksToUse.insert(std::pair<int,int>(allocatedBlock, tempBlockNum));
							blocksNeeded -= tempBlockNum;
							tempBlockNum = blocksNeeded;
							entriesUsed++;
						}
					}
					// File can be supported on the virtual disk.
					int blockLen = 0;
					int indexBlockOffset = 0;
					cout << "Adding file " << fileName << " with index block at " << indexBlock << endl;
					cout << "Added file " << fileName << " at ";
					// Proceed to save to disk data
					while(blocksToUse.empty() == false){
						// Get the first allocated block number
						allocatedBlock = blocksToUse.begin()->first;
						// Get the first length of the block
						blockLen = blocksToUse.begin()->second;
						// Calculate position of allocated block in disk data
						allocatedBlockStartingAddr = (allocatedBlock*DiskVCB->blockSize) - DiskVCB->blockSize;
						// Loop for totol length of the blocks
						for(int i=0;i < (blockLen*DiskVCB->blockSize);i++){
							if (fileContents.size() != 0){
								// For every new 'block'
								if (i % DiskVCB->blockSize == 0){
									// Print block details
									cout << "B" << DiskData[allocatedBlockStartingAddr+i].block << "(";
									cout << fileContents.front();
									if(fileContents.size() > 1){
										cout << ",";
									} else {
										cout << ") ";
									}
								} else {
									// Print file data
									cout << fileContents.front();
									if(fileContents.size() > 1 && i % DiskVCB->blockSize != DiskVCB->blockSize-1){
										cout << ",";
									} else {
										cout << ") ";
									}
								}
								// Write file entry to disk data
								DiskData[allocatedBlockStartingAddr+i].data = stoi(fileContents.front());
								// Increment access time (Accessed memory)
								accessTime++;
								// Delete file entry from the queue
								fileContents.pop();
							}
						}
						// Save the block number into index block
						DiskData[(indexBlock*DiskVCB->blockSize)-DiskVCB->blockSize+indexBlockOffset].data = allocatedBlock;
						// Increment access time (Accessed memory)
						accessTime++;
						// Save the block length into index block
						DiskData[(indexBlock*DiskVCB->blockSize)-DiskVCB->blockSize+indexBlockOffset].length = blockLen;
						// Increment access time (Accessed memory)
						accessTime++;
						// Increment index block offset
						indexBlockOffset++;
						// Delete the block data from our map
						blocksToUse.erase(allocatedBlock);
					}
					// Update the file's inode in the directory structure
					updateINode(tempDirPtr, fileName, indexBlock);
					// Increment access time (Accessed memory)
					accessTime++;
				}
				cout << endl;
				break;
			}
		}	
		cout << "Total access time (accesses to memory) is " << accessTime << endl << endl;	
		return 1;
	} else {
		cout << "Adding file " << fileName << "." << endl;		
		cout << "Error: File system unable to support any more files." << endl << endl;
		return 0;
	}
}




/*
 * Reads file data from virtual disk
 *	This method checks if the requested file to be read exists in the file
 *  directory. If yes, the file will be read. If the file does not exists,
 *	an error is printed and the method ends.
 *
 * Returns:
 *		'1' when completed successfully
 *		'0' if unsuccessful
 *		
 */
int VirtualDisk::readFile(int fileName){
	int accessTime = 0;
	// Get main file name by deducting the remainder of the modulus of 100
	int mainFileID = fileName - (fileName % 100);
	// Get the file offset within the file
	int fileOffset = fileName % 100;
	// Check if file name exists in the directory structure
	iNode* tempDirPtr = checkINode(mainFileID);
	// Increment access time (Accessed memory)
	accessTime++;
	if (tempDirPtr){
		// File found
		switch (allocationMethod){				
			case (1): {
				// Allocation method 1 - Contiguous
				if(fileOffset > 0){
					// File data entry selected
					cout << "Read File " << mainFileID << "(" << fileName << ")" << " from virtual disk." << endl;
					if(fileOffset <= tempDirPtr->Length * DiskVCB->blockSize){
						// Calculate direct position of the entry in disk data
						int tempAddr = (tempDirPtr->StartBlock * DiskVCB->blockSize)-DiskVCB->blockSize+fileOffset-1;
						if(DiskData[tempAddr].data != -1){
							// If there is data at the entry, print the data entry details
							cout << "Located at block: " << DiskData[tempAddr].block << ", index: " << DiskData[tempAddr].index << " with data: " << DiskData[tempAddr].data << endl;
						} else {
							// No data found, print error message and exit method
							cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
							return 0;
						}
						// Increment access time (Accessed memory)
						accessTime++;
					} else {
						// File offset is out of allocated boundaries, print error and exit method
						cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
						return 0;
					}

				} else {
					// Direct file number given (print whole file)
					cout << "Read File " << fileName << " from virtual disk." << endl;
					cout << "File " << mainFileID << " is stored from block " << tempDirPtr->StartBlock << " to " << tempDirPtr->StartBlock + tempDirPtr->Length-1 << "." << endl;
					cout << "File data: ";
					// Calculate direct starting position of the block
					int tempAddr = (tempDirPtr->StartBlock * DiskVCB->blockSize)-DiskVCB->blockSize;
					for(int i=0;i< tempDirPtr->Length*DiskVCB->blockSize;i++){
						// Loop through the entries in the blocks
						if (DiskData[tempAddr+i].data != -1){
							// If there is data at the entry, print the data entry details
							cout << DiskData[tempAddr+i].data << " ";
						}
						// Increment access time (Accessed memory)
						accessTime++;
					}
					cout << endl;
				}
				break;
			}
			case (2): {
				// Allocation method 2 - Linked
				if(fileOffset > 0){				
					// Print data entry
					cout << "Read File " << mainFileID << "(" << fileName << ")" << " from virtual disk." << endl;
					// Calculate direct position of starting 'head' block in disk data
 					int tempAddr = (tempDirPtr->StartBlock * DiskVCB->blockSize)-DiskVCB->blockSize;
 					// Calcualte the number of blocks to jump ahead based on the offset
					int jumps = floor((fileOffset-1) / (DiskVCB->blockSize-1));
					for (int i=0;i<jumps;i++){
						// Jump ahead (reduce unnecessary accesses to memory)
						if(DiskData[tempAddr].block != tempDirPtr->LastBlock){
							// Calculate offset of the next block directly
							tempAddr = ((DiskData[tempAddr+(DiskVCB->blockSize-1)].data) * DiskVCB->blockSize) - DiskVCB->blockSize;
							// Update the remaining number of offsets
							fileOffset -= (DiskVCB->blockSize-1);
						} else {
							if(i < jumps){
								// Current block is the last block specified in the inode but we have more jumps to do, print error and exit method.
								cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
								return 0;								
							}
						}
						// Increment access time (Accessed memory)
						accessTime++;
					}
					// Target block found, add the remaining offset within the block 
					tempAddr += fileOffset-1;
					if (DiskData[tempAddr].data == -1){
						// No data found, print error and exit method
						cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
						return 0;
					} else {
						// Entry has data, print data entry details
						cout << "Located at block: " << DiskData[tempAddr].block << ", index: " << DiskData[tempAddr].index << " with data: " << DiskData[tempAddr].data << endl;
					}
					// Increment access time (Accessed memory)
					accessTime++;
				} else {
					// Direct file number given (print whole file)
					cout << "Read File " << fileName << " from virtual disk." << endl;
					cout << "File " << mainFileID << " starts at block " << tempDirPtr->StartBlock << " and ends at block " << tempDirPtr->LastBlock << "." << endl;
					cout << "File data: ";
					// Calculate direct position of starting 'head' block in disk data
					int tempAddr = (tempDirPtr->StartBlock * DiskVCB->blockSize)-DiskVCB->blockSize;
					// Initialise internal block offset
					int offset = 0;
					// While data entry is not empty
					while (DiskData[tempAddr+offset].data != -1){
						// At every last entry of the block, get next block info
						if (offset > 0 && (offset % (DiskVCB->blockSize)) == (DiskVCB->blockSize-1)){
							// Calculate direct position of the next block
							tempAddr = (DiskData[tempAddr+offset].data * DiskVCB->blockSize)-DiskVCB->blockSize;
							// Reset internal block offset 
							offset = 0;
						} else {
							// Print data entry details
							cout << DiskData[tempAddr+offset].data << " ";
							// Increment internal block offset
							offset++;
						}
						// Increment access time (Accessed memory)
						accessTime++;
					}
					cout << endl;
				}
				break;
			}
			case (3): {
				// Allocation method 3 - Indexed
				int indexBlockStartingAddr = 0;
				if(fileOffset > 0){
					// File data entry selected
					cout << "Read File " << mainFileID << "(" << fileName << ")" << " from virtual disk." << endl;
					// Calculate number of offset within index block
					int jumps = floor((fileOffset-1)/DiskVCB->blockSize);
					// Update remaining offset within data block
					fileOffset -= (jumps*DiskVCB->blockSize)+1;
					// Calculate direct position of index block in disk data
 					indexBlockStartingAddr = ((tempDirPtr->Index)*DiskVCB->blockSize)-DiskVCB->blockSize;
 					// If jumps required within index block is greater than number of entries in it
					if(jumps > DiskVCB->blockSize){
						// File offset is out of allocated boundaries of index block, print error and exit method
						cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
						return 0;
					} else {
						// Valid number of jumps within index block
						if(DiskData[indexBlockStartingAddr+jumps].data == -1){
							// Selected entry in index block is empty, prin error and exit method
							cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
							return 0;
						} else {
							// Entry in index block has data, save the block number
							int tempBlock = DiskData[indexBlockStartingAddr+jumps].data;
							// Calculate direct position of the block in disk data
							int tempAddr = (tempBlock*DiskVCB->blockSize)-DiskVCB->blockSize+fileOffset;
							if(DiskData[tempAddr].data != -1){
								// File entry has data, print disk data details
								cout << "Located at block: " << DiskData[tempAddr].block << ", index: " << DiskData[tempAddr].index << " with data: " << DiskData[tempAddr].data << endl;
							} else {
								// Selected offset in data block is empty, print error and exit method
								cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
								return 0;
							}
							// Increment access time (Accessed memory)
							accessTime++;
						}
						// Increment access time (Accessed memory)
						accessTime++;
					}
				} else {
					// Direct file number given (print whole file)
					int allocatedBlockStartingAddr = 0;
					queue<int> indexNumbers;
					// Calculate direct position of index block in disk data
					indexBlockStartingAddr = ((tempDirPtr->Index - 1)*DiskVCB->blockSize);
					// Increment access time (Accessed memory)
					accessTime++;
					for (int i = 0; i < DiskVCB->blockSize; i++){
						// Go through each entry in the index block
						if(DiskData[indexBlockStartingAddr + i].data != -1){
							// Index block entry has data, push block number to queue
							indexNumbers.push(DiskData[indexBlockStartingAddr + i].data);
						}
						// Increment access time (Accessed memory)
						accessTime++;
					}
					cout << "Read File " << fileName << " from virtual disk." << endl;
					cout << "File " << mainFileID << "\'s index block is at block " << tempDirPtr->Index << "." << endl;
					cout << "File data: ";
					// Process every entry in the queue
					while(indexNumbers.size() > 0){
						// Calculate direct position of the first block in the front of the queue
						allocatedBlockStartingAddr = (((indexNumbers.front()-1)*DiskVCB->blockSize)+1);
						// Loop through each entry in the block
						for(int i = 0; i < DiskVCB->blockSize; i++){
							if (DiskData[allocatedBlockStartingAddr+i-1].data != -1){
								// Print data entry details
								cout << DiskData[(allocatedBlockStartingAddr+i-1)].data << " ";
							}
							// Increment access time (Accessed memory)
							accessTime++;
						}
						// Remove ('dequeue') first item from the queue
						indexNumbers.pop();
					}
					cout << endl;
				}
				break;
			}
			case (4): {
				// Allocation method 4 - Contiguous indexed
				int indexBlockStartingAddr = 0;
				if(fileOffset > 0){
					// File data entry selected
					cout << "Read File " << mainFileID << "(" << fileName << ")" << " from virtual disk." << endl;
					// Calculate number of blocks required to jump ahead to the offset
					int jumps = floor((fileOffset-1)/DiskVCB->blockSize);
					// Update new offset within data block
					fileOffset -= (jumps*DiskVCB->blockSize)+1;
					// Set start of index block
 					indexBlockStartingAddr = ((tempDirPtr->Index)*DiskVCB->blockSize)-DiskVCB->blockSize;
					// Increment access time (Accessed memory)
					accessTime++;
 					// Set offset within index block
 					int indexBlockOffset = 0;
 					while(jumps > 0){
						// If number of jumps is greater than current entry's length in index block
						if(jumps - DiskData[indexBlockStartingAddr+indexBlockOffset].length >= 0){
							// Deduct the number of jumps required
	 						jumps -= DiskData[indexBlockStartingAddr+indexBlockOffset].length; 						
						} else {
							// Our data entry is within this index block entry
							break;
						}
						// Increment access time (Accessed memory)
						accessTime++;
						// Move pointer to the next entry in the index block
						indexBlockOffset++;
 						if(indexBlockOffset > DiskVCB->blockSize-1){
 							// If we moved out of the index block's limits, return error and exit method.
 							cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
							return 0;
 						}
 					}
 					// Calculate direct position to the tart of the target contiguous block
 					int tempBlockAddr = (DiskData[indexBlockStartingAddr+indexBlockOffset].data*DiskVCB->blockSize)-DiskVCB->blockSize;
					// Increment access time (Accessed memory)
					accessTime++;
 					//Calculate offset within the contiguous block
 					fileOffset += (jumps*DiskVCB->blockSize);
 					if(fileOffset > (DiskData[indexBlockStartingAddr+indexBlockOffset].length*DiskVCB->blockSize)-1){
 						// Entry requested is out of bounds of the blocks, return error and exit method
 						cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
						return 0;
 					}
					// Increment access time (Accessed memory)
					accessTime++;
 					if(DiskData[tempBlockAddr+fileOffset].data != -1){
 						// Print data entry details
						cout << "Located at block: " << DiskData[tempBlockAddr+fileOffset].block << ", index: " << DiskData[tempBlockAddr+fileOffset].index << " with data: " << DiskData[tempBlockAddr+fileOffset].data << endl;
 					} else {
 						// File entry is empty, print error and exit method.
 						cout << "Error: File " << mainFileID << "(" << fileName << ") cannot be found on the virtual disk." << endl << endl;
 						return 0;
 					}
					// Increment access time (Accessed memory)
					accessTime++;
				} else {
					// Direct file number given (print whole file)
					int allocatedBlockStartingAddr = 0;
					// Initialise a queue of data pairs
					queue<std::pair<int,int> > indexNumbers;
					// Calculate direct position of index block in disk data
					indexBlockStartingAddr = ((tempDirPtr->Index - 1)*DiskVCB->blockSize);
					// Increment access time (Accessed memory)
					accessTime++;
					// Loop through each entry in the index block
					for (int i = 0; i < DiskVCB->blockSize; i++){
						if(DiskData[indexBlockStartingAddr + i].data != -1){
							// Push the block and length as a pair into the queue
							indexNumbers.push(std::pair<int,int>(DiskData[indexBlockStartingAddr + i].data, DiskData[indexBlockStartingAddr + i].length));
						}
						// Increment access time (Accessed memory)
						accessTime++;
					}
					cout << "Read File " << fileName << " from virtual disk." << endl;
					cout << "File " << mainFileID << "\'s index block is at block " << tempDirPtr->Index << "." << endl;
					cout << "File data: ";
					// Process every entry in the queue
					while(indexNumbers.size() > 0){
						// Calculate direct position of the first block in the front of the queue
						allocatedBlockStartingAddr = (((indexNumbers.front().first-1)*DiskVCB->blockSize)+1);
						for(int i = 0; i < indexNumbers.front().second*DiskVCB->blockSize; i++){
							// Loop through each entry in the block
							if (DiskData[allocatedBlockStartingAddr+i-1].data != -1){
								// Print data entry details
								cout << DiskData[(allocatedBlockStartingAddr+i-1)].data << " ";
							}
							// Increment access time (Accessed memory)
							accessTime++;
						}
						// Remove ('dequeue') first item from the queue
						indexNumbers.pop();
					}
					cout << endl;
				}
				break;
			}
		}
		cout << "Total access time (accesses to memory) is " << accessTime << endl << endl;	
		return 1;
	} else {
		// File not found in the directory structure, print error and exit method
		cout << "Read File " << mainFileID << "(" << fileName << ")" << " from virtual disk." << endl;
		cerr << "Error: File entry "<< mainFileID << "(" << fileName << ")" << " does not exist in the virtual disk." << endl << endl;
		return 0;
	}
	return 1;
}




/*
 *  Deletes file data from virtual disk
 *	This method checks if the requested file to be deleted exists in the file
 *  directory. If yes, the file will be deleted. If the file does not exists,
 *	an error is printed and the method ends.
 *
 * Returns:
 *		'1' when completed successfully
 *		'0' if unsuccessful
 *		
 */
int VirtualDisk::deleteFile(int fileName){
	int accessTime = 0;
	iNode* tempDirPtr;
	// Check if file name exists in the directory structure
	tempDirPtr = checkINode(fileName);
	// Increment access time (Accessed memory)
	accessTime++;
	if (tempDirPtr){
		// File found
		switch (allocationMethod){				
			case (1): {
				// Allocation method 1 - Contiguous
				cout << "Deleting File " << fileName << " from virtual disk." << endl;
				cout << "Deleted File " << fileName << " from virtual disk and freed ";
				// Calculate direct position of the block in disk data  
				int tempAddr = (tempDirPtr->StartBlock * DiskVCB->blockSize)-DiskVCB->blockSize;
				// Increment access time (Accessed memory)
				accessTime++;
				for(int i=0;i< tempDirPtr->Length*DiskVCB->blockSize;i++){
					// Loop through all entries in the contiguous blocks
					if (i % DiskVCB->blockSize == 0){
						// Print block number
						cout << "B" << DiskData[tempAddr+i].block << " ";
					}
					if (DiskData[tempAddr+i].data != -1){
						// If there is data in the entry, set to -1 (unused)
						DiskData[tempAddr+i].data = -1;
					}
					// Increment access time (Accessed memory)
					accessTime++;
				}
				// Update the free space bit map + number of free blocks
				updateFreeSpace(tempDirPtr->StartBlock, tempDirPtr->Length, 1);
				// Update the inode in directory structure to -1 (unused)
				updateINode(tempDirPtr,-1,-1);
				// Increment access time (Accessed memory)
				accessTime++;
				cout << endl;
				break;
			}
			case (2): {
				// Allocation method 2 - Linked
				cout << "Deleting File " << fileName << " from virtual disk." << endl;
				cout << "Deleted File " << fileName << " from virtual disk and freed ";
				// Calculate direct position of the block in disk data
				int tempAddr = (tempDirPtr->StartBlock * DiskVCB->blockSize)-DiskVCB->blockSize;
				// Increment access time (Accessed memory)
				accessTime++;
				int offset = 0;
				int tempBlock = 0;
				// While data entry is not empty
				while (DiskData[tempAddr+offset].data != -1){
					// At every last entry of the block
					if ((offset % (DiskVCB->blockSize)) == (DiskVCB->blockSize-1)){
						// Print block number
						cout << "B" << DiskData[tempAddr+offset].block << " ";
						// Update the free space bit map + number of free blocks
						updateFreeSpace(DiskData[tempAddr+offset].block, 1, 1);
						// Get next block number
						tempBlock = tempAddr+offset;
						// Calculate direct position of the next block in disk data
						tempAddr = (DiskData[tempAddr+offset].data * DiskVCB->blockSize)-DiskVCB->blockSize;
						// Delete the entry in the current block
						DiskData[tempBlock].data = -1;
						// Reset internal offset within the block
						offset = 0;
					} else {
						// If there is data in the entry, set to -1 (unused)
						DiskData[tempAddr+offset].data = -1;
						// Increase internal offset within the block
						offset++;
					}
					// Increment access time (Accessed memory)
					accessTime++;

				}
				// Print block number
				cout << "B" << DiskData[tempAddr+offset].block << " ";
				// Update the free space bit map + number of free blocks
				updateFreeSpace(DiskData[tempAddr+offset].block, 1, 1);
				// Update the inode in directory structure to -1 (unused)
				updateINode(tempDirPtr,-1,-1);
				// Increment access time (Accessed memory)
				accessTime++;
				cout << endl;
				break;
			}
			case (3): {
				// Allocation method 3 - Indexed
				cout << "Deleting File " << fileName << " from virtual disk." << endl;
				cout << "Deleted File " << fileName << " from virtual disk and freed ";
				// Direct file number
				int indexBlockStartingAddr = 0;
				int allocatedBlockStartingAddr = 0;
				queue<int> indexNumbers;
				// Calculate direct position of index block in disk data
				indexBlockStartingAddr = ((tempDirPtr->Index - 1)*DiskVCB->blockSize);
				// Increment access time (Accessed memory)
				accessTime++;
				for (int i = 0; i < DiskVCB->blockSize; i++){
					// Go through each entry in the index block
					if(DiskData[indexBlockStartingAddr + i].data != -1){
						// Index block entry has data, push block number to queue
						indexNumbers.push(DiskData[indexBlockStartingAddr + i].data);

						DiskData[indexBlockStartingAddr+i].data = -1;
					}
					// Increment access time (Accessed memory)
					accessTime++;
				}
				cout << "B" << DiskData[indexBlockStartingAddr].block << " ";
				// Update the free space bit map + number of free blocks
				updateFreeSpace(DiskData[indexBlockStartingAddr].block, 1, 1);
				while(indexNumbers.size() > 0){
					allocatedBlockStartingAddr = (((indexNumbers.front()-1)*DiskVCB->blockSize)+1);
					for(int i = 0; i < DiskVCB->blockSize; i++){
						if (DiskData[allocatedBlockStartingAddr+i-1].data != -1){
							// If there is data in the entry, set to -1 (unused)
							DiskData[(allocatedBlockStartingAddr+i-1)].data = -1;
						}
						// Increment access time (Accessed memory)
						accessTime++;

					}
					// Print block number
					cout << "B" << indexNumbers.front() << " ";
					// Update the free space bit map + number of free blocks
					updateFreeSpace(indexNumbers.front(), 1, 1);
					// Remove ('dequeue') first item from the queue
					indexNumbers.pop();
				}
				// Update the inode in directory structure to -1 (unused)
				updateINode(tempDirPtr,-1,-1);
				// Increment access time (Accessed memory)
				accessTime++;
				cout << endl;
				break;
			}
			case (4): {
				cout << "Deleting File " << fileName << " from virtual disk." << endl;
				cout << "Deleted File " << fileName << " from virtual disk and freed ";
				int indexBlockStartingAddr = 0;
				int allocatedBlockStartingAddr = 0;
				// Initialise a queue of data pairs
				queue<std::pair<int,int> > indexNumbers;
				// Calculate direct position of index block in disk data
				indexBlockStartingAddr = ((tempDirPtr->Index - 1)*DiskVCB->blockSize);
				// Increment access time (Accessed memory)
				accessTime++;
				for (int i = 0; i < DiskVCB->blockSize; i++){
					// Loop through each entry in the index block
					if(DiskData[indexBlockStartingAddr + i].data != -1){
						// Push the bock and length as a pair into the queue
						indexNumbers.push(std::pair<int,int>(DiskData[indexBlockStartingAddr + i].data, DiskData[indexBlockStartingAddr + i].length));
						// Write index block entry data to -1 (unused)
						DiskData[indexBlockStartingAddr+i].data = -1;
						// Write index block entry length to -1 (unused)
						DiskData[indexBlockStartingAddr+i].length = -1;
					}
					// Increment access time (Accessed memory)
					accessTime++;
				}
				// Update the free space bit map + number of free blocks
				updateFreeSpace(DiskData[indexBlockStartingAddr].block, 1, 1);
				// Print blokc number
				cout << "B" << DiskData[indexBlockStartingAddr].block << " ";
				// Process every entry in the queue
				while(indexNumbers.size() > 0){
					// Calculate direct position of the first block in the front of the queue
					allocatedBlockStartingAddr = (((indexNumbers.front().first-1)*DiskVCB->blockSize)+1);
					for(int i = 0; i < indexNumbers.front().second*DiskVCB->blockSize; i++){
						// Loop each entry in the block
						if (DiskData[allocatedBlockStartingAddr+i-1].data != -1){
							// If there is data in the entry, set to -1 (unused)
							DiskData[(allocatedBlockStartingAddr+i-1)].data = -1;
						}
						if (i % DiskVCB->blockSize == 0){
							// Print block number for every new block accessed
							cout << "B" << DiskData[allocatedBlockStartingAddr+i].block << " ";
						}
						// Increment access time (Accessed memory)
						accessTime++;
					}
					// Update the free space bit map + number of free blocks
					updateFreeSpace(DiskData[allocatedBlockStartingAddr].block, indexNumbers.front().second, 1);
					// Remove ('dequeue') first item from the queue
					indexNumbers.pop();
				}

				// Update the inode in directory structure to -1 (unused)
				updateINode(tempDirPtr,-1,-1);	
				// Increment access time (Accessed memory)
				accessTime++;
				cout << endl;
				break;
			}
		}
		cout << "Total access time (accesses to memory) is " << accessTime << endl << endl;	
	} else {
		// File not found in the directory structure, print error and exit method
		cout << "Deleting File " << fileName << "(" << fileName << ")" << " from virtual disk." << endl;
		cerr << "Error: File entry "<< fileName << "(" << fileName << ")" << " does not exist in the virtual disk." << endl << endl;
		return 0;
	}
	return 1;
}







/*
 * Getter method for the disk's VCB
 *
 * Returns:
 *		Disk VCB pointer
 *
 */
VCB* VirtualDisk::getVCB(){
	return DiskVCB;
}


/*
 * Getter method for the disk data
 *
 * Returns:
 *		Disk Data structure pointer
 *
 */


Data* VirtualDisk::getData(){
	return DiskData;
}


/*
 * Getter method for blockSize
 *
 * Returns:
 *		Block size for the Disk
 *
 */
int VirtualDisk::getBlockSize(){
	return DiskVCB->blockSize;
}



/*
 * Getter method for allocation method
 *
 * Returns:
 *		'1' for Contiguous
 *		'2' for Linked
 *		'3' for Indexed
 *		'4' for Contiguous Indexed
 *
 */
int VirtualDisk::getAllocationMethod(){
	return allocationMethod;
}


