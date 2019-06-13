#ifndef _bplusTree_
#define _bplusTree_

#include <string>
#include <cstring>
#include <math.h>
#include <vector>
#include <iostream>
#include <utility>
using namespace std;
#define blockSize 4096 // 块大小规定为4KB，这里和缓冲区相同
#define nodeSize 3
#define INT_MAX 1000000
#define MAX 300 // Math.floor((4096-4-4)/12)

class bNode
{
public:
    int size;
    bNode *parentNode;
    bNode *childNode[MAX]; // 从0开始，共MAX个基本单位
    pair<int,int> value[MAX]; // first:key; second:addr/offset

    bNode();
    bool isLeaf();
};

bNode::bNode()
{
    size = 0;
    parentNode = nullptr;
    for (int i = 0; i < MAX; i++)
    {
        value[i] = make_pair(INT_MAX,-1);
        childNode[i] = nullptr;
    }
}
bool bNode::isLeaf()
{
    return childNode[0] == nullptr;
}

class bplusTree
{
public:
    int numberOfPointers;
    bool dataFound;       //每次删除节点时记得把datafound更新为false
    bool searchDataFound; //每次搜索节点时记得把searchDataFound更新为false
    string tableName;     // 对应的表名
    string keyName;       // 对应的属性名
    bNode *root;          // 树顶点

    bplusTree();
    ~bplusTree(){};
    bplusTree(string tableName, string keyName);

    void splitLeaf(bNode *curNode);
    void splitNonLeaf(bNode *curNode);
    void insertNode(bNode *curNode, int key, int addr);
    void deleteNode(bNode *curNode, int key, int curNodePosition);
    void mergeNode(bNode *leftNode, bNode *rightNode, bool isLeaf, int posOfRightNode);
    void redistributeNode(bNode *leftNode, bNode *rightNode, bool isLeaf, int posOfLeftNode, int whichOneisCurNode);
    int getAddrWithKey(bNode *curNode, int key);

    static void print(vector<bNode *> Nodes); // 打印树结构，调试用
};

bplusTree::bplusTree()
{
    numberOfPointers = nodeSize + 1;
    root = new bNode();
    searchDataFound = false;
    dataFound = false;
};

bplusTree::bplusTree(string tableName, string keyName)
{
    this->tableName = tableName;
    this->keyName = keyName;
    root = new bNode();
    numberOfPointers = nodeSize + 1;
    searchDataFound = false;
    dataFound = false;
}

void bplusTree::print(vector<bNode *> Nodes)
{
    vector<bNode *> newNodes;
    for (auto curNode : Nodes)
    {
        cout << "[|";
        int j;
        for (j = 0; j < curNode->size; j++)
        {
            cout << curNode->value[j].first<<":"<<curNode->value[j].second << "|";
            if (curNode->childNode[j] != nullptr)
                newNodes.push_back(curNode->childNode[j]);
        }
        if (curNode->value[j].first == INT_MAX && curNode->childNode[j] != nullptr)
            newNodes.push_back(curNode->childNode[j]);
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

void bplusTree::splitLeaf(bNode *curNode)
{
//    cout << "this is split leaf node function\n";
    pair<int,int> tempPair;
    int x, i, j;

    if (numberOfPointers % 2)
        x = (numberOfPointers + 1) / 2;
    else
        x = numberOfPointers / 2;

    auto *rightNode = new bNode();

    curNode->size = x;
    rightNode->size = numberOfPointers - x;
    rightNode->parentNode = curNode->parentNode;

    for (i = x, j = 0; i < numberOfPointers; i++, j++)
    {
        rightNode->value[j] = curNode->value[i];
        curNode->value[i] = make_pair(INT_MAX,-1);
    }
    int val = rightNode->value[0].first;
    int address = rightNode->value[0].second;
    if (curNode->parentNode == nullptr)
    {
        auto *parentNode = new bNode();
        parentNode->parentNode = nullptr;
        parentNode->size = 1;
        parentNode->value[0] = rightNode->value[0];
        parentNode->childNode[0] = curNode;
        parentNode->childNode[1] = rightNode;
        curNode->parentNode = rightNode->parentNode = parentNode;
        root = parentNode;
        return;
    }
    else
    {
        curNode = curNode->parentNode;
        auto *newChildNode = new bNode();
        newChildNode = rightNode;
        tempPair = make_pair(val,address);
        for (i = 0; i <= curNode->size; i++)
        {
            if (val < curNode->value[i].first)
            {
                swap(curNode->value[i], tempPair);
            }
        }
        curNode->size++;
        for (i = 0; i < curNode->size; i++)
        {
            if (newChildNode->value[0] < curNode->childNode[i]->value[0])
            {
                swap(curNode->childNode[i], newChildNode);
            }
        }
        curNode->childNode[i] = newChildNode;
        for (i = 0; curNode->childNode[i] != nullptr; i++)
        {
            curNode->childNode[i]->parentNode = curNode;
        }
    }
}

void bplusTree::splitNonLeaf(bNode *curNode)
{
    cout << "this is split nonleaf node function\n";
    int x, i, j;
    pair<int,int> tempPair;

    x = numberOfPointers / 2;
    auto *rightNode = new bNode();
    curNode->size = x;
    rightNode->size = numberOfPointers - x - 1;
    rightNode->parentNode = curNode->parentNode;

    for (i = x, j = 0; i <= numberOfPointers; i++, j++)
    {
        rightNode->value[j] = curNode->value[i];
        rightNode->childNode[j] = curNode->childNode[i];
        curNode->value[i] = make_pair(INT_MAX,-1);
        if (i != x)
            curNode->childNode[i] = nullptr;
    }
    int val = rightNode->value[0].first;
    int address = rightNode->value[0].second;
    memcpy(&rightNode->value, &rightNode->value[1], sizeof(int) * (rightNode->size + 1));
    memcpy(&rightNode->childNode, &rightNode->childNode[1], sizeof(root) * (rightNode->size + 1));

    for (i = 0; curNode->childNode[i] != nullptr; i++)
    {
        curNode->childNode[i]->parentNode = curNode;
    }
    for (i = 0; rightNode->childNode[i] != nullptr; i++)
    {
        rightNode->childNode[i]->parentNode = rightNode;
    }

    if (curNode->parentNode == nullptr)
    {
        auto *parentNode = new bNode();
        parentNode->parentNode = nullptr;
        parentNode->size = 1;
        parentNode->value[0] = make_pair(val,address);
        parentNode->childNode[0] = curNode;
        parentNode->childNode[1] = rightNode;
        curNode->parentNode = rightNode->parentNode = parentNode;
        root = parentNode;
        return;
    }
    else
    {
        curNode = curNode->parentNode;
        auto *newChildNode = new bNode();
        newChildNode = rightNode;
        tempPair = make_pair(val,address);
        for (i = 0; i <= curNode->size; i++)
        {
            if (val < curNode->value[i].first)
            {
                swap(curNode->value[i], tempPair);
            }
        }
        curNode->size++;
        for (i = 0; i < curNode->size; i++)
        {
            if (newChildNode->value[0] < curNode->childNode[i]->value[0])
            {
                swap(curNode->childNode[i], newChildNode);
            }
        }
        curNode->childNode[i] = newChildNode;
        for (i = 0; curNode->childNode[i] != nullptr; i++)
        {
            curNode->childNode[i]->parentNode = curNode;
        }
    }
}

void bplusTree::insertNode(bNode *curNode, int key, int addr)
{
    cout << "this is insert node function\n";
    pair<int,int> tempPair = make_pair(key,addr);
    for (int i = 0; i <= curNode->size; i++)
    {
        if (key < curNode->value[i].first && curNode->childNode[i] != nullptr)
        {
            insertNode(curNode->childNode[i], key,addr);
            if (curNode->size == numberOfPointers)
                splitNonLeaf(curNode);
            return;
        }
        else if (key < curNode->value[i].first && curNode->childNode[i] == nullptr)
        {
            swap(curNode->value[i], tempPair);
            if (i == curNode->size)
            {
                curNode->size++;
                break;
            }
        }
    }
    if (curNode->size == numberOfPointers)
    {
        splitLeaf(curNode);
    }
}

void bplusTree::redistributeNode(bNode *leftNode, bNode *rightNode, bool isLeaf, int posOfLeftNode, int whichOneisCurNode)
{
    if (whichOneisCurNode == 0)
    {
        if (!isLeaf)
        {
            leftNode->value[leftNode->size] = leftNode->parentNode->value[posOfLeftNode];
            leftNode->childNode[leftNode->size + 1] = rightNode->childNode[0];
            leftNode->size++;
            leftNode->parentNode->value[posOfLeftNode] = rightNode->value[0];
            memcpy(&rightNode->value[0], &rightNode->value[1], sizeof(int) * (rightNode->size + 1));
            memcpy(&rightNode->childNode[0], &rightNode->childNode[1], sizeof(root) * (rightNode->size + 1));
            rightNode->size--;
        }
        else
        {
            leftNode->value[leftNode->size] = rightNode->value[0];
            leftNode->size++;
            memcpy(&rightNode->value[0], &rightNode->value[1], sizeof(int) * (rightNode->size + 1));
            rightNode->size--;
            leftNode->parentNode->value[posOfLeftNode] = rightNode->value[0];
        }
    }
    else
    {
        if (!isLeaf)
        {
            memcpy(&rightNode->value[1], &rightNode->value[0], sizeof(int) * (rightNode->size + 1));
            memcpy(&rightNode->childNode[1], &rightNode->childNode[0], sizeof(root) * (rightNode->size + 1));
            rightNode->value[0] = leftNode->parentNode->value[posOfLeftNode];
            rightNode->childNode[0] = leftNode->childNode[leftNode->size];
            rightNode->size++;
            leftNode->parentNode->value[posOfLeftNode] = leftNode->value[leftNode->size - 1];
            leftNode->value[leftNode->size - 1] = make_pair(INT_MAX,-1);
            leftNode->childNode[leftNode->size] = nullptr;
            leftNode->size--;
        }
        else
        {
            memcpy(&rightNode->value[1], &rightNode->value[0], sizeof(int) * (rightNode->size + 1));
            rightNode->value[0] = leftNode->value[leftNode->size - 1];
            rightNode->size++;
            leftNode->value[leftNode->size - 1] = make_pair(INT_MAX,-1);
            leftNode->size--;
            leftNode->parentNode->value[posOfLeftNode] = rightNode->value[0];
        }
    }
}

void bplusTree::mergeNode(bNode *leftNode, bNode *rightNode, bool isLeaf, int posOfRightNode)
{
    if (!isLeaf)
    {
        leftNode->value[leftNode->size] = leftNode->parentNode->value[posOfRightNode - 1];
        leftNode->size++;
    }
    memcpy(&leftNode->value[leftNode->size], &rightNode->value[0], sizeof(int) * (rightNode->size + 1));
    memcpy(&leftNode->childNode[leftNode->size], &rightNode->childNode[0], sizeof(root) * (rightNode->size + 1));

    leftNode->size += rightNode->size;
    memcpy(&leftNode->parentNode->value[posOfRightNode - 1], &leftNode->parentNode->value[posOfRightNode], sizeof(int) * (leftNode->parentNode->size + 1));
    memcpy(&leftNode->parentNode->childNode[posOfRightNode], &leftNode->parentNode->childNode[posOfRightNode + 1], sizeof(root) * (leftNode->parentNode->size + 1));
    leftNode->parentNode->size--;
    for (int i = 0; leftNode->childNode[i] != nullptr; i++)
    {
        leftNode->childNode[i]->parentNode = leftNode;
    }
}

void bplusTree::deleteNode(bNode *curNode, int key, int curNodePosition)
{
    bool isLeaf = curNode->isLeaf();
    int prevLeftMostVal = curNode->value[0].first;

    for (int i = 0; !dataFound && i <= curNode->size; i++)
    {
        if (key < curNode->value[i].first && curNode->childNode[i] != nullptr)
        {
            deleteNode(curNode->childNode[i], key, i);
        }
        else if (key == curNode->value[i].first && curNode->childNode[i] == nullptr)
        {
            memcpy(&curNode->value[i], &curNode->value[i + 1], sizeof(int) * (curNode->size + 1));
            curNode->size--;
            dataFound = true;
            break;
        }
    }
    if (curNode->parentNode == nullptr && curNode->childNode[0] == nullptr)
    {
        return;
    }
    if (curNode->parentNode == nullptr && curNode->childNode[0] != nullptr && curNode->size == 0)
    {
        root = curNode->childNode[0];
        root->parentNode = nullptr;
        return;
    }
    if (isLeaf && curNode->parentNode != nullptr)
    {
        if (curNodePosition == 0)
        {
            auto *rightNode = new bNode();
            rightNode = curNode->parentNode->childNode[1];
            if (rightNode != nullptr && rightNode->size > (numberOfPointers + 1) / 2)
            {
                redistributeNode(curNode, rightNode, isLeaf, 0, 0);
            }
            else if (rightNode != nullptr && curNode->size + rightNode->size < numberOfPointers)
            {
                mergeNode(curNode, rightNode, isLeaf, 1);
            }
        }
        else
        {
            auto *leftNode = new bNode();
            auto *rightNode = new bNode();
            leftNode = curNode->parentNode->childNode[curNodePosition - 1];
            rightNode = curNode->parentNode->childNode[curNodePosition + 1];
            if (leftNode != nullptr && leftNode->size > (numberOfPointers + 1) / 2)
            {
                redistributeNode(leftNode, curNode, isLeaf, curNodePosition - 1, 1);
            }
            else if (rightNode != nullptr && rightNode->size > (numberOfPointers + 1) / 2)
            {
                redistributeNode(curNode, rightNode, isLeaf, curNodePosition, 0);
            }
            else if (leftNode != nullptr && curNode->size + leftNode->size < numberOfPointers)
            {
                mergeNode(leftNode, curNode, isLeaf, curNodePosition);
            }
            else if (rightNode != nullptr && curNode->size + rightNode->size < numberOfPointers)
            {
                mergeNode(curNode, rightNode, isLeaf, curNodePosition + 1);
            }
        }
    }
    else if (!isLeaf && curNode->parentNode != nullptr)
    {

        if (curNodePosition == 0)
        {
            auto *rightNode = new bNode();
            rightNode = curNode->parentNode->childNode[1];
            if (rightNode != nullptr && rightNode->size - 1 >= ceil((numberOfPointers - 1) / 2))
            {
                redistributeNode(curNode, rightNode, isLeaf, 0, 0);
            }
            else if (rightNode != nullptr && curNode->size + rightNode->size < numberOfPointers - 1)
            {
                mergeNode(curNode, rightNode, isLeaf, 1);
            }
        }
        else
        {
            auto *leftNode = new bNode();
            auto *rightNode = new bNode();
            leftNode = curNode->parentNode->childNode[curNodePosition - 1];
            rightNode = curNode->parentNode->childNode[curNodePosition + 1];
            if (leftNode != nullptr && leftNode->size - 1 >= ceil((numberOfPointers - 1) / 2))
            {
                redistributeNode(leftNode, curNode, isLeaf, curNodePosition - 1, 1);
            }
            else if (rightNode != nullptr && rightNode->size - 1 >= ceil((numberOfPointers - 1) / 2))
            {
                redistributeNode(curNode, rightNode, isLeaf, curNodePosition, 0);
            }
            else if (leftNode != nullptr && curNode->size + leftNode->size < numberOfPointers - 1)
            {
                mergeNode(leftNode, curNode, isLeaf, curNodePosition);
            }
            else if (rightNode != nullptr && curNode->size + rightNode->size < numberOfPointers - 1)
            {
                mergeNode(curNode, rightNode, isLeaf, curNodePosition + 1);
            }
        }
    }
    auto *tempNode = new bNode();
    tempNode = curNode->parentNode;
    while (tempNode != nullptr)
    {
        for (int i = 0; i < tempNode->size; i++)
        {
            if (tempNode->value[i].first == prevLeftMostVal)
            {
                tempNode->value[i] = curNode->value[0];
                break;
            }
        }
        tempNode = tempNode->parentNode;
    }
}

int bplusTree::getAddrWithKey(bNode *curNode, int key)
{
    for (int i = 0; i < curNode->size; i++)
    {
        if (curNode->value[i].first == key)
        {
            searchDataFound = true;
            return curNode->value[i].second;
        }
        else if (curNode->value[i].first > key)
        {
            if(curNode->isLeaf()) return -1;
            return getAddrWithKey(curNode->childNode[i], key);
        }
    }
    if(!searchDataFound) return -1;
}

#endif