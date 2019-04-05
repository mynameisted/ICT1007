#ifndef HEADER_H
#define HEADER_H
#pragma once

// Preprocessor Directives
#include <iostream> 	// IO Streams
#include <iomanip>		// Output Formatting (set width)
#include <string> 		// String & array
#include <sstream> 		// String Streams
#include <fstream> 		// File Streams
#include <locale>		// std::locale, std::tolower
#include <climits>		// C limits
#include <stdbool.h> 	// Boolean 
#include <math.h> 		// Math
#include <algorithm>	// Algorithms
#include <queue>		// Queues
#include <vector>		// Vectors
#include <map>			// Maps

// Compiler Directive
using namespace std;


/*
 * Global Variables
 */
// Forward declaration of MAX_ELEMENTS - Declared in Main.cpp
extern const int MAX_ELEMENTS;

/*
 *
 * Data Structures
 *
 */
// STRUCT FOR VOLUME CONTROL BLOCK {SUPERBLOCK FOR UNIX FILE SYSTEM | MASTER FILE TABLE FOR NEW TECHNOLOGY FILE SYSTEM (NTFS USED BY WINDOWS)}
typedef struct VCB {
	int totalBlockNum;				// Total number of blocks
	int numFreeBlock;				// Number of free blocks
	int blockSize;					// Block size
	vector<int> FreeBlockBitMap;	// Free Block Bit map array
	int allocationMethod;			// Allocation method of volume (1 - Contiguous, 2 - Linked, 3 - Indexed, 4 - Contiguous Indexed)
} VCB; 


// STRUCT FOR INODE
typedef struct iNode {
	int FileIdentifier; // File Identifier/File name [Series of integer range from 100, 200, 300..9900]
	int StartBlock;		// Start block
	int LastBlock;		// Last block
	int Length;			// Number of blocks including start block
	int Index;			// Index block
} iNode; 

// STRUCT FOR DATA
typedef struct {
	int index;
	int block;
	int data;
	int length;
} Data;

#endif
