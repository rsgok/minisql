#include "bplusTree.h"

void testTree();
int main()
{
    testTree();
}

void testTree()
{
    int mode, key;
    bplusTree mytree;
    vector<bNode *> Nodes;
    while (1)
    {
        cin >> mode >> key;
        if (mode == 1)
        {
            cout << "insert " << key << "\n";
            mytree.insertNode(mytree.root, key);
        }
        if (mode == 2)
        {
            cout << "delete " << key << "\n";
            mytree.dataFound = false;
            mytree.deleteNode(mytree.root, key, 0);
        }
        Nodes.push_back(mytree.root);
        mytree.print(Nodes);
        Nodes.clear();
    }
}