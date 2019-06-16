#include "MyBufferManager.h"
#include "Catalog.h"
#include <stdio.h>

// constructor
BufferManager::BufferManager(/* args */)
{
    for (int i = 0; i < MAXBLOCKNUM; i++)
    {
        blocks[i].Init();
    }
}

// destructor
BufferManager::~BufferManager()
{
    for (int i = 0; i < MAXBLOCKNUM; i++)
        if (blocks[i].isOccupied)
            WriteBack(i);
}

// write back the block to file (if it's modified)
void BufferManager::WriteBack(int blockAddr)
{
    // if this block is not modified, do nothing
    if (!blocks[blockAddr].isModified)
        return;
    // if it's modified, write data back to file
    string fileName = blocks[blockAddr].fileName;
    FILE *fp;
	if ((fp = fopen(fileName.c_str(), "r+b")) == NULL) {
		cout << "Open file error!" << endl; 
		return;
	}
    fseek(fp, BLOCKSIZE * blocks[blockAddr].blockOffset, SEEK_SET);
    fwrite(blocks[blockAddr].data, BLOCKSIZE, 1, fp);

    // then, reset this block
    blocks[blockAddr].Init();
    fclose(fp);
}

// get the address of block in buffer (if not in buffer, it'll be automatically pushed to buffer)
int BufferManager::getBlockAddr(string fileName, int blockOffset)
{
    // first, check if it's in the buffer
    int blockAddr = checkExist(fileName, blockOffset);

    // if not exist, push the block from file to buffer.
    if (blockAddr == -1)
    {
        // find a unoccupied block
        blockAddr = getUnoccupiedBlock();
        // push it to buffer
        readBlock(fileName, blockOffset, blockAddr);
    }

    blocks[blockAddr].recent_access_time = clock();

    return blockAddr;
}

// check if the block is in buffer, if exist, return address. if not, return -1
int BufferManager::checkExist(string fileName, int blockOffset)
{
    for (int blockAddr = 0; blockAddr < MAXBLOCKNUM; blockAddr++)
    {
        if (blocks[blockAddr].fileName == fileName && blocks[blockAddr].blockOffset == blockOffset)
            return blockAddr;
    }
    return -1;
}

// find the unoccupied or LRU block to be overwritten; return its address
int BufferManager::getUnoccupiedBlock()
{
    int LRUaddr = 0; // the address of LRU block
    for (int i = 0; i < MAXBLOCKNUM; i++)
    {
        // if unoccupied block exist
        if (!blocks[i].isOccupied)
        {
            blocks[i].Init();
            blocks[i].isOccupied = 1;
            // return address
            return i;
        }
        // find the LRU time
        else if (blocks[LRUaddr].recent_access_time > blocks[i].recent_access_time)
        {
            LRUaddr = i;
        }
    }

    // write back the LRU block
    WriteBack(LRUaddr);
    blocks[LRUaddr].isOccupied = 1;
    //return address
    return LRUaddr;
}

// read block from files to buffer
void BufferManager::readBlock(string fileName, int blockOffset, int blockAddr)
{
    blocks[blockAddr].isOccupied = 1;
    blocks[blockAddr].isModified = 0;
    blocks[blockAddr].fileName = fileName;
    blocks[blockAddr].blockOffset = blockOffset;
    blocks[blockAddr].recent_access_time = clock();

    FILE *fp;
    fp = fopen(fileName.c_str(), "rb");
    fseek(fp, BLOCKSIZE * blockOffset, SEEK_SET);
    fread(blocks[blockAddr].data, BLOCKSIZE, 1, fp);
    fclose(fp);
}

// if you want to modify the block in buffer, you must call this function first
void BufferManager::modifyBlock(int blockAddr)
{
    blocks[blockAddr].isModified = 1;
    // record the time
    blocks[blockAddr].recent_access_time = clock();
}

// return the position where you can insert new data
InsPos BufferManager::getInsertPosition(Table &tableinfor)
{
    InsPos iPos;
    if (tableinfor.blockNum == 0)
    { //new file and no block exist
        iPos.blockAddr = addBlockInFile(tableinfor);
        iPos.position = 0;
        return iPos;
    }
    string filename = tableinfor.getname() + ".table";
    int length = tableinfor.dataSize() + 1;    // first bit as an additional one to check if occupied
    int blockOffset = tableinfor.blockNum - 1; // put the latest-added element at end
    int blockAddr = checkExist(filename, blockOffset);
    if (blockAddr == -1)
    {
        blockAddr = getUnoccupiedBlock();
        readBlock(filename, blockOffset, blockAddr);
    }
    int recordNum = BLOCKSIZE / length;
    for (int offset = 0; offset < recordNum; offset++)
    {
        int position = offset * length;
        char isEmpty = blocks[blockAddr].data[position]; //check if there are contents
        if (isEmpty == EMPTY)
        { //find an empty space
            iPos.blockAddr = blockAddr;
            iPos.position = position;
            return iPos;
        }
    }
    //if the block is full, then create new block
    iPos.blockAddr = addBlockInFile(tableinfor);
    iPos.position = 0;

    for (int i = 0; i < MAXBLOCKNUM; i++)
        if (blocks[i].isOccupied)
            WriteBack(i);

    return iPos;
}

// add new block into file and return its adress in buffer
int BufferManager::addBlockInFile(Table &tableinfor)
{
    int blockAddr = getUnoccupiedBlock();
    blocks[blockAddr].Init();
    blocks[blockAddr].isOccupied = 1;
    blocks[blockAddr].isModified = 1;
    blocks[blockAddr].fileName = tableinfor.getname() + ".table";
    blocks[blockAddr].blockOffset = tableinfor.blockNum++;
    blocks[blockAddr].recent_access_time = clock();
    CataManager ca;
    ca.changeblock(tableinfor.getname(), tableinfor.blockNum);
    return blockAddr;
}

void BufferManager::setInvalid(string fileName)
{
    for (int i = 0; i < MAXBLOCKNUM; i++)
    {
        if (blocks[i].fileName == fileName)
        {
            blocks[i].isOccupied = 0;
            blocks[i].isModified = 0;
        }
    }
}
