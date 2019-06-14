#ifndef _MYBUFFERMANAGER_H
#define _MYBUFFERMANAGER_H

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <time.h>
#include "base.h"
#include "const.h"

using namespace std;


// position to insert
struct InsPos
{
    int blockAddr; // block address
    int position;  // position in the block
};

// the structure of blocks in buffer
class BufferBlock
{
private:
    /* data */
public:

    bool isOccupied;  // check if this block is un-used
    bool isModified;  // check if this block has been modified
    string fileName;  // the name of file to which the block belongs to
    int blockOffset;  // to locate the position of block in original file
    int recent_access_time; // to tell when the block is called
    char data[BLOCKSIZE + 1]; // data stored in the block 

    BufferBlock();
    ~BufferBlock(){};

    void Init();
    string getDataStr(int begin, int end);
    char getDataChar(int position);

};

BufferBlock::BufferBlock()
{
    Init();
}

// Initialize the block
void BufferBlock::Init()
{
    isModified = 0;
    isOccupied = 0;
    fileName = "";
    blockOffset = 0;
    memset(data, EMPTY, BLOCKSIZE);
    data[BLOCKSIZE] = '\0';
}

// get data in string form 
string BufferBlock::getDataStr(int begin, int end)
{
    string res = "";
    for (int i = begin; i < end; i++)
    {
        res += data[i];
    }
    return res;
}

// get data in char form 
char BufferBlock::getDataChar(int position)
{
    return data[position];
}

class BufferManager
{
private:
    /* data */
public:



    BufferManager(/* args */);
    ~BufferManager();

    void WriteBack(int blockAddr);
    int getBlockAddr(string fileName, int blockOffset);
    int checkExist(string fileName, int blockOffset);
    int getUnoccupiedBlock();
    void readBlock(string fileName, int blockOffset, int blockAddr);
    void modifyBlock(int blockAddr);
    InsPos getInsertPosition(Table &tableinfor);
    int addBlockInFile(Table &tableinfor);

    BufferBlock blocks[MAXBLOCKNUM];

    friend class RecordManager;
    friend class CataManager;
};

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
    {
        for (int i = 0; i < MAXBLOCKNUM; i++)
        {
            WriteBack(i);
        }
    }
}

// write back the block to file (if it's modified)
void BufferManager::WriteBack(int blockAddr)
{
    // if this block is not modified, do nothing
    if(!blocks[blockAddr].isModified)
        return;
    // if it's modified, write data back to file
    string fileName = blocks[blockAddr].fileName;
    FILE *fp;
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
    if(blockAddr == -1)
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
    int LRUaddr = 0;  // the address of LRU block
    for (int i = 0; i < MAXBLOCKNUM; i++)
    {
        // if unoccupied block exist
        if(!blocks[i].isOccupied)
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
    // if ((fp = fopen(fileName.c_str(), "rb")) == NULL)
    // {
    //     cout << "Open file error" << endl;
    //     return;
    // }
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
    int length = tableinfor.dataSize() + 1;    //多余的一位放在开头，表示是否有效
    int blockOffset = tableinfor.blockNum - 1; //将新加入的元素插入到最后
    int blockAddr = checkExist(filename, blockOffset);
    if (blockAddr == -1)
    {
        blockAddr = getUnoccupiedBlock(); //获取空的block
        readBlock(filename, blockOffset, blockAddr);
    }
    int recordNum = BLOCKSIZE / length;
    for (int offset = 0; offset < recordNum; offset++)
    {
        int position = offset * length;
        char isEmpty = blocks[blockAddr].data[position]; //检查第一位是否有效，判断该行是否有内容
        if (isEmpty == EMPTY)
        { //find an empty space
            iPos.blockAddr = blockAddr;
            iPos.position = position;
            return iPos;
        }
    }
    //该block已经装满，新开一个block
    iPos.blockAddr = addBlockInFile(tableinfor);
    iPos.position = 0;
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

#endif