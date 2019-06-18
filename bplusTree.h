#ifndef _bplusTree_
#define _bplusTree_

#include <string>
#include <cstring>
#include <math.h>
#include <vector>
#include <iostream>
#include <utility>
#include <fstream>
#include "MyBufferManager.hpp"
using namespace std;
#define blockSize 4096 // 块大小规定为4KB，这里和缓冲区相同
#define nodeSize 3
#define INT_MAX 1000000
#define MAX 300 // Math.floor((4096-4-4)/12)

extern BufferManager bf;

class bNode
{
public:
    int size;
    int parentNode;
    int childNode[MAX];        // 从0开始，共MAX个基本单位
    pair<int, int> value[MAX]; // first:key; second:addr/offset

    bNode();
    bool isLeaf();
};

bNode::bNode()
{
    size = 0;
    parentNode = -1;
    for (int i = 0; i < MAX; i++)
    {
        value[i] = make_pair(INT_MAX, -1);
        childNode[i] = -1;
    }
}
bool bNode::isLeaf()
{
    return childNode[0] == -1;
}

class bplusTree
{
public:
    int numberOfPointers;
    bool dataFound;       //每次删除节点时记得把datafound更新为false
    bool searchDataFound; //每次搜索节点时记得把searchDataFound更新为false
    string tableName;     // 对应的表名
    int root;             // 树顶点

    bplusTree(string tableName);

    void splitLeaf(int curNode);
    void splitNonLeaf(int curNode);
    void insertNode(int curNode, int key, int addr);
    void deleteNode(int curNode, int key, int curNodePosition);
    void mergeNode(int leftNode, int rightNode, bool isLeaf, int posOfRightNode);
    void redistributeNode(int leftNode, int rightNode, bool isLeaf, int posOfLeftNode, int whichOneisCurNode);
    int getAddrWithKey(int curNode, int key);
    bNode *getBlockPtr(int curNode);

    void print(vector<bNode *> Nodes); // 打印树结构，调试用
};

bplusTree::bplusTree(string tableName)
{
    this->tableName = tableName;
    root = 0;
    numberOfPointers = nodeSize + 1;
    searchDataFound = false;
    dataFound = false;
    fstream fp;
    fp.open(tableName, ios::in);
    if (!fp)
    {
        fp.open(tableName, ios::out);
        return;
    }
}

bNode *bplusTree::getBlockPtr(int curNode)
{
    int buffernum = bf.getBlockAddr(tableName, curNode);
    return (bNode *)bf.blocks[buffernum].data;
}

void bplusTree::print(vector<bNode *> Nodes)
{
    vector<bNode *> newNodes;
    bNode *tempNode;
    for (auto curNode : Nodes)
    {
        cout << "[|";
        int j;
        for (j = 0; j < curNode->size; j++)
        {
            cout << curNode->value[j].first << ":" << curNode->value[j].second << "|";
            if (curNode->childNode[j] != -1)
                newNodes.push_back(getBlockPtr(curNode->childNode[j]));
        }
        if (curNode->value[j].first == INT_MAX && curNode->childNode[j] != -1)
            newNodes.push_back(getBlockPtr(curNode->childNode[j]));
        cout << "]  ";
    }
    if (newNodes.empty())
    {

        puts("");
        puts("");
        Nodes.clear();
    }
    else
    {
        puts("");
        puts("");
        Nodes.clear();
        print(newNodes);
    }
}

void bplusTree::splitLeaf(int curNode)
{
    bf.modifyBlock(curNode);
    //    cout << "this is split leaf node function\n";
    pair<int, int> tempPair;
    int x, i, j;
    bNode *node = getBlockPtr(curNode);

    if (numberOfPointers % 2)
        x = (numberOfPointers + 1) / 2;
    else
        x = numberOfPointers / 2;

    int newBlockNum_rightNode = bf.getUnoccupiedBlock();
    bNode *rightNode = getBlockPtr(newBlockNum_rightNode);
    bf.modifyBlock(newBlockNum_rightNode);

    node->size = x;
    rightNode->size = numberOfPointers - x;
    rightNode->parentNode = node->parentNode;

    for (i = x, j = 0; i < numberOfPointers; i++, j++)
    {
        rightNode->value[j] = node->value[i];
        node->value[i] = make_pair(INT_MAX, -1);
    }
    int val = rightNode->value[0].first;
    int address = rightNode->value[0].second;
    if (node->parentNode == -1)
    {
        int newBlockNum_parentNode = bf.getUnoccupiedBlock();
        bNode *parentNode = getBlockPtr(newBlockNum_parentNode);
        bf.modifyBlock(newBlockNum_parentNode);
        parentNode->parentNode = -1;
        parentNode->size = 1;
        parentNode->value[0] = rightNode->value[0];
        parentNode->childNode[0] = curNode;
        parentNode->childNode[1] = newBlockNum_rightNode;
        node->parentNode = rightNode->parentNode = newBlockNum_parentNode;
        root = newBlockNum_parentNode;
        return;
    }
    else
    {
        curNode = node->parentNode;
        int newBlockNum_ChildNode = bf.getUnoccupiedBlock();
        bNode *newChildNode = getBlockPtr(newBlockNum_ChildNode);
        bf.modifyBlock(newBlockNum_ChildNode);
        newChildNode = rightNode;
        tempPair = make_pair(val, address);
        for (i = 0; i <= node->size; i++)
        {
            if (val < node->value[i].first)
            {
                swap(node->value[i], tempPair);
            }
        }
        node->size++;
        bNode *tempBlockPtr;
        for (i = 0; i < node->size; i++)
        {
            tempBlockPtr = getBlockPtr(node->childNode[i]);
            if (newChildNode->value[0] < tempBlockPtr->value[0])
            {
                swap(node->childNode[i], newBlockNum_ChildNode);
            }
        }
        node->childNode[i] = newBlockNum_ChildNode;
        for (i = 0; node->childNode[i] != -1; i++)
        {
            tempBlockPtr = getBlockPtr(node->childNode[i]);
            tempBlockPtr->parentNode = curNode;
        }
    }
}

void bplusTree::splitNonLeaf(int curNode)
{
    bf.modifyBlock(curNode);
    // cout << "this is split nonleaf node function\n";
    int x, i, j;
    pair<int, int> tempPair;
    bNode *node = getBlockPtr(curNode);

    x = numberOfPointers / 2;

    int newBlockNum_rightNode = bf.getUnoccupiedBlock();
    bNode *rightNode = getBlockPtr(newBlockNum_rightNode);
    bf.modifyBlock(newBlockNum_rightNode);

    node->size = x;
    rightNode->size = numberOfPointers - x - 1;
    rightNode->parentNode = node->parentNode;

    for (i = x, j = 0; i <= numberOfPointers; i++, j++)
    {
        rightNode->value[j] = node->value[i];
        rightNode->childNode[j] = node->childNode[i];
        node->value[i] = make_pair(INT_MAX, -1);
        if (i != x)
            node->childNode[i] = -1;
    }
    int val = rightNode->value[0].first;
    int address = rightNode->value[0].second;
    memcpy(&rightNode->value, &rightNode->value[1], sizeof(int) * (rightNode->size + 1));
    memcpy(&rightNode->childNode, &rightNode->childNode[1], sizeof(root) * (rightNode->size + 1));

    bNode *tempBlockPtr;
    for (i = 0; node->childNode[i] != -1; i++)
    {
        tempBlockPtr = getBlockPtr(node->childNode[i]);
        tempBlockPtr->parentNode = curNode;
    }
    for (i = 0; rightNode->childNode[i] != -1; i++)
    {
        tempBlockPtr = getBlockPtr(node->childNode[i]);
        tempBlockPtr->parentNode = newBlockNum_rightNode;
    }

    if (node->parentNode == -1)
    {
        int newBlockNum_parentNode = bf.getUnoccupiedBlock();
        bNode *parentNode = getBlockPtr(newBlockNum_parentNode);
        bf.modifyBlock(newBlockNum_parentNode);
        parentNode->parentNode = -1;
        parentNode->size = 1;
        parentNode->value[0] = make_pair(val, address);
        parentNode->childNode[0] = curNode;
        parentNode->childNode[1] = newBlockNum_rightNode;
        node->parentNode = rightNode->parentNode = newBlockNum_parentNode;
        root = newBlockNum_parentNode;
        return;
    }
    else
    {
        curNode = node->parentNode;
        int newBlockNum_ChildNode = bf.getUnoccupiedBlock();
        bNode *newChildNode = getBlockPtr(newBlockNum_ChildNode);
        bf.modifyBlock(newBlockNum_ChildNode);
        newChildNode = rightNode;
        tempPair = make_pair(val, address);
        for (i = 0; i <= node->size; i++)
        {
            if (val < node->value[i].first)
            {
                swap(node->value[i], tempPair);
            }
        }
        node->size++;
        for (i = 0; i < node->size; i++)
        {
            tempBlockPtr = getBlockPtr(node->childNode[i]);
            if (newChildNode->value[0] < tempBlockPtr->value[0])
            {
                swap(node->childNode[i], newBlockNum_ChildNode);
            }
        }
        node->childNode[i] = newBlockNum_ChildNode;
        for (i = 0; node->childNode[i] != -1; i++)
        {
            tempBlockPtr = getBlockPtr(node->childNode[i]);
            tempBlockPtr->parentNode = curNode;
        }
    }
}

void bplusTree::insertNode(int curNode, int key, int addr)
{
    bf.modifyBlock(curNode);
    // cout << "this is insert node function\n";
    pair<int, int> tempPair = make_pair(key, addr);
    bNode *node = getBlockPtr(curNode);
    for (int i = 0; i <= node->size; i++)
    {
        if (key < node->value[i].first && node->childNode[i] != -1)
        {
            insertNode(node->childNode[i], key, addr);
            if (node->size == numberOfPointers)
                splitNonLeaf(curNode);
            return;
        }
        else if (key < node->value[i].first && node->childNode[i] == -1)
        {
            swap(node->value[i], tempPair);
            if (i == node->size)
            {
                node->size++;
                break;
            }
        }
    }
    if (node->size == numberOfPointers)
    {
        splitLeaf(curNode);
    }
}

void bplusTree::redistributeNode(int leftNode, int rightNode, bool isLeaf, int posOfLeftNode, int whichOneisCurNode)
{
    bNode *tempBlockPtr;
    bf.modifyBlock(leftNode);
    bf.modifyBlock(rightNode);
    bNode *leftNodePtr = getBlockPtr(leftNode);
    bNode *rightNodePtr = getBlockPtr(rightNode);
    tempBlockPtr = getBlockPtr(leftNodePtr->parentNode);
    if (whichOneisCurNode == 0)
    {
        if (!isLeaf)
        {
            leftNodePtr->value[leftNodePtr->size] = tempBlockPtr->value[posOfLeftNode];
            leftNodePtr->childNode[leftNodePtr->size + 1] = rightNodePtr->childNode[0];
            leftNodePtr->size++;
            tempBlockPtr->value[posOfLeftNode] = rightNodePtr->value[0];
            memcpy(&rightNodePtr->value[0], &rightNodePtr->value[1], sizeof(int) * (rightNodePtr->size + 1));
            memcpy(&rightNodePtr->childNode[0], &rightNodePtr->childNode[1], sizeof(root) * (rightNodePtr->size + 1));
            rightNodePtr->size--;
        }
        else
        {
            leftNodePtr->value[leftNodePtr->size] = rightNodePtr->value[0];
            leftNodePtr->size++;
            memcpy(&rightNodePtr->value[0], &rightNodePtr->value[1], sizeof(int) * (rightNodePtr->size + 1));
            rightNodePtr->size--;
            tempBlockPtr->value[posOfLeftNode] = rightNodePtr->value[0];
        }
    }
    else
    {
        if (!isLeaf)
        {
            memcpy(&rightNodePtr->value[1], &rightNodePtr->value[0], sizeof(int) * (rightNodePtr->size + 1));
            memcpy(&rightNodePtr->childNode[1], &rightNodePtr->childNode[0], sizeof(root) * (rightNodePtr->size + 1));
            rightNodePtr->value[0] = tempBlockPtr->value[posOfLeftNode];
            rightNodePtr->childNode[0] = leftNodePtr->childNode[leftNodePtr->size];
            rightNodePtr->size++;
            tempBlockPtr->value[posOfLeftNode] = leftNodePtr->value[leftNodePtr->size - 1];
            leftNodePtr->value[leftNodePtr->size - 1] = make_pair(INT_MAX, -1);
            leftNodePtr->childNode[leftNodePtr->size] = -1;
            leftNodePtr->size--;
        }
        else
        {
            memcpy(&rightNodePtr->value[1], &rightNodePtr->value[0], sizeof(int) * (rightNodePtr->size + 1));
            rightNodePtr->value[0] = leftNodePtr->value[leftNodePtr->size - 1];
            rightNodePtr->size++;
            leftNodePtr->value[leftNodePtr->size - 1] = make_pair(INT_MAX, -1);
            leftNodePtr->size--;
            tempBlockPtr->value[posOfLeftNode] = rightNodePtr->value[0];
        }
    }
}

void bplusTree::mergeNode(int leftNode, int rightNode, bool isLeaf, int posOfRightNode)
{
    bNode *tempBlockPtr;
    bNode *tempBlockPtr2;
    bf.modifyBlock(leftNode);
    bf.modifyBlock(rightNode);
    bNode *leftNodePtr = getBlockPtr(leftNode);
    bNode *rightNodePtr = getBlockPtr(rightNode);
    tempBlockPtr = getBlockPtr(leftNodePtr->parentNode);
    if (!isLeaf)
    {
        leftNodePtr->value[leftNodePtr->size] = tempBlockPtr->value[posOfRightNode - 1];
        leftNodePtr->size++;
    }
    memcpy(&leftNodePtr->value[leftNodePtr->size], &rightNodePtr->value[0], sizeof(int) * (rightNodePtr->size + 1));
    memcpy(&leftNodePtr->childNode[leftNodePtr->size], &rightNodePtr->childNode[0], sizeof(root) * (rightNodePtr->size + 1));

    leftNodePtr->size += rightNodePtr->size;
    memcpy(&tempBlockPtr->value[posOfRightNode - 1], &tempBlockPtr->value[posOfRightNode], sizeof(int) * (tempBlockPtr->size + 1));
    memcpy(&tempBlockPtr->childNode[posOfRightNode], &tempBlockPtr->childNode[posOfRightNode + 1], sizeof(root) * (tempBlockPtr->size + 1));
    tempBlockPtr->size--;
    for (int i = 0; leftNodePtr->childNode[i] != -1; i++)
    {
        tempBlockPtr2 = getBlockPtr(leftNodePtr->childNode[i]);
        tempBlockPtr2->parentNode = leftNode;
    }
}

void bplusTree::deleteNode(int curNode, int key, int curNodePosition)
{
    bf.modifyBlock(curNode);
    bNode *node = getBlockPtr(curNode);
    bNode *tempBlockPtr;

    bool isLeaf = node->isLeaf();
    int prevLeftMostVal = node->value[0].first;

    for (int i = 0; !dataFound && i <= node->size; i++)
    {
        if (key < node->value[i].first && node->childNode[i] != -1)
        {
            deleteNode(node->childNode[i], key, i);
        }
        else if (key == node->value[i].first && node->childNode[i] == -1)
        {
            memcpy(&node->value[i], &node->value[i + 1], sizeof(int) * (node->size + 1));
            node->size--;
            dataFound = true;
            break;
        }
    }
    if (node->parentNode == -1 && node->childNode[0] == -1)
    {
        return;
    }
    if (node->parentNode == -1 && node->childNode[0] != -1 && node->size == 0)
    {
        root = node->childNode[0];
        bf.modifyBlock(root);
        tempBlockPtr = getBlockPtr(root);
        tempBlockPtr->parentNode = -1;
        return;
    }
    if (isLeaf && node->parentNode != -1)
    {
        if (curNodePosition == 0)
        {
            tempBlockPtr = getBlockPtr(node->parentNode);
            int rightNodeNum = tempBlockPtr->childNode[1];
            bNode *rightNode = getBlockPtr(rightNodeNum);
            if (rightNodeNum != -1 && rightNode->size > (numberOfPointers + 1) / 2)
            {
                redistributeNode(curNode, rightNodeNum, isLeaf, 0, 0);
            }
            else if (rightNodeNum != -1 && node->size + rightNode->size < numberOfPointers)
            {
                mergeNode(curNode, rightNodeNum, isLeaf, 1);
            }
        }
        else
        {
            tempBlockPtr = getBlockPtr(node->parentNode);
            int leftNodeNum = tempBlockPtr->childNode[curNodePosition - 1];
            int rightNodeNum = tempBlockPtr->childNode[curNodePosition + 1];
            bNode *leftNode = getBlockPtr(leftNodeNum);
            bNode *rightNode = getBlockPtr(rightNodeNum);
            if (leftNodeNum != -1 && leftNode->size > (numberOfPointers + 1) / 2)
            {
                redistributeNode(leftNodeNum, curNode, isLeaf, curNodePosition - 1, 1);
            }
            else if (rightNodeNum != -1 && rightNode->size > (numberOfPointers + 1) / 2)
            {
                redistributeNode(curNode, rightNodeNum, isLeaf, curNodePosition, 0);
            }
            else if (leftNodeNum != -1 && node->size + leftNode->size < numberOfPointers)
            {
                mergeNode(leftNodeNum, curNode, isLeaf, curNodePosition);
            }
            else if (rightNodeNum != -1 && node->size + rightNode->size < numberOfPointers)
            {
                mergeNode(curNode, rightNodeNum, isLeaf, curNodePosition + 1);
            }
        }
    }
    else if (!isLeaf && node->parentNode != -1)
    {

        if (curNodePosition == 0)
        {
            tempBlockPtr = getBlockPtr(node->parentNode);
            int rightNodeNum = tempBlockPtr->childNode[1];
            bNode *rightNode = getBlockPtr(rightNodeNum);
            if (rightNodeNum != -1 && rightNode->size - 1 >= ceil((numberOfPointers - 1) / 2))
            {
                redistributeNode(curNode, rightNodeNum, isLeaf, 0, 0);
            }
            else if (rightNodeNum != -1 && node->size + rightNode->size < numberOfPointers - 1)
            {
                mergeNode(curNode, rightNodeNum, isLeaf, 1);
            }
        }
        else
        {
            tempBlockPtr = getBlockPtr(node->parentNode);
            int leftNodeNum = tempBlockPtr->childNode[curNodePosition - 1];
            int rightNodeNum = tempBlockPtr->childNode[curNodePosition + 1];
            bNode *leftNode = getBlockPtr(leftNodeNum);
            bNode *rightNode = getBlockPtr(rightNodeNum);
            if (leftNodeNum != -1 && leftNode->size - 1 >= ceil((numberOfPointers - 1) / 2))
            {
                redistributeNode(leftNodeNum, curNode, isLeaf, curNodePosition - 1, 1);
            }
            else if (rightNodeNum != -1 && rightNode->size - 1 >= ceil((numberOfPointers - 1) / 2))
            {
                redistributeNode(curNode, rightNodeNum, isLeaf, curNodePosition, 0);
            }
            else if (leftNodeNum != -1 && node->size + leftNode->size < numberOfPointers - 1)
            {
                mergeNode(leftNodeNum, curNode, isLeaf, curNodePosition);
            }
            else if (rightNodeNum != -1 && node->size + rightNode->size < numberOfPointers - 1)
            {
                mergeNode(curNode, rightNodeNum, isLeaf, curNodePosition + 1);
            }
        }
    }
    int tempNodeNum = node->parentNode;
    bNode *tempNode = getBlockPtr(tempNodeNum);
    while (tempNodeNum != -1)
    {
        for (int i = 0; i < tempNode->size; i++)
        {
            if (tempNode->value[i].first == prevLeftMostVal)
            {
                tempNode->value[i] = node->value[0];
                break;
            }
        }
        tempNodeNum = tempNode->parentNode;
        tempNode = getBlockPtr(tempNodeNum);
    }
}

int bplusTree::getAddrWithKey(int curNode, int key)
{
    bNode *node = getBlockPtr(curNode);
    for (int i = 0; i < node->size; i++)
    {
        if (node->value[i].first == key)
        {
            searchDataFound = true;
            return node->value[i].second;
        }
        else if (node->value[i].first > key)
        {
            if (node->isLeaf())
                return -1;
            return getAddrWithKey(node->childNode[i], key);
        }
    }
    if (!searchDataFound)
        return -1;
}

#endif