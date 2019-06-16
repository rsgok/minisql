#ifndef _MYBUFFERMANAGER_H
#define _MYBUFFERMANAGER_H
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
    bool isOccupied;          // check if this block is un-used
    bool isModified;          // check if this block has been modified
    string fileName;          // the name of file to which the block belongs to
    int blockOffset;          // to locate the position of block in original file
    int recent_access_time;   // to tell when the block is called
    char data[BLOCKSIZE + 1]; // data stored in the block

    BufferBlock()
    {
        Init();
    }
    ~BufferBlock()
    {
    }

    void Init()
    {
        isModified = 0;
        isOccupied = 0;
        fileName = "";
        blockOffset = 0;
        memset(data, EMPTY, BLOCKSIZE);
        data[BLOCKSIZE] = '\0';
    }
    string getDataStr(int begin, int end)
    {
        string res = "";
        for (int i = begin; i < end; i++)
        {
            res += data[i];
        }
        return res;
    }
    char getDataChar(int position)
    {
        return data[position];
    }
};


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
    void setInvalid(string fileName);

    BufferBlock blocks[MAXBLOCKNUM];

    friend class RecordManager;
    friend class CataManager;
};

#endif