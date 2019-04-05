// VirtualDisk.h
#ifndef VIRTUALDISK_H
#define VIRTUALDISK_H
#pragma once

#define printDiskMapWidth 10

class VirtualDisk{
	VCB* DiskVCB;
	iNode* DiskDir;
	Data* DiskData;
	int allocationMethod;
public:
	// Constructors/Destructors
	VirtualDisk();
	~VirtualDisk();
	// General methods
	int setupDisk();
	int setupDiskData();
	int setupDiskDir();
	int setBlockSize();
	int setAllocationMethod();
	int buildFSBitMap();

	// Getter Methods
	VCB* getVCB();
	Data* getData();
	int getBlockSize();
	int getAllocationMethod();

	// File operation methods
	int addFile(int fileName, queue<string> args);
	int readFile(int fileName);
	int deleteFile(int fileName);
	// Utility methods	
	int requestBlocks(int numBlocks);
	iNode* checkINode(int);
	int updateINode(iNode* iNodeEntry, int fileName, int startValue, int endValue);
	int updateFreeSpace(int startValue, int valueLen, int newValue);

	// Printing methods
	void printVCB();
	void printDiskMap();
	void printFreeSpaceBitMap();
};

#endif