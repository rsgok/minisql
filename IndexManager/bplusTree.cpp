#include "bplusTree.h"

void testTree();
int main()
{
    testTree();
}

void testTree()
{
    int mode, key, addr;
    bplusTree mytree;
    vector<bNode *> Nodes;
    while (1)
    {
        cin >> mode >> key;
        if (mode == 1)
        {
            cout << "insert " << key << "\n";
            cin >> addr;
            cout << "addr: " << addr << "\n";
            mytree.insertNode(mytree.root, key,addr);
            Nodes.push_back(mytree.root);
            mytree.print(Nodes);
            Nodes.clear();
        }
        if (mode == 2)
        {
            cout << "delete " << key << "\n";
            mytree.dataFound = false;
            mytree.deleteNode(mytree.root, key, 0);
            Nodes.push_back(mytree.root);
            mytree.print(Nodes);
            Nodes.clear();
        }
        if (mode == 3)
        {
            cout << "search " << key << "\n";
            mytree.searchDataFound = false;
            int temp= mytree.getAddrWithKey(mytree.root, key);
            cout <<  temp << "\n";
        }
    }
}