#include "client.h"

Client *flattenedClientTree;
int numClients;

// walkTree will walk the specified client tree, calling the provided function
// on each client, and passing in the provided void * each call.
void walkTree(Client tree, void (*walkFunc) (Client, void *), void *data) {
    walkFunc(tree, data);
    for(int i = 0; i < tree.numChildren; i++) {
        walkTree(tree.children[i], walkFunc, data);
    }
}

// clientCounter, when provided to walkTree, will count up the number of
// clients. The void * is expected to point to an int.
void clientCounter(Client c, void *data) {
    int *num = (int *)data;
    *num = *num + 1;
};

// flattener will, when provided to walkTree, will add each client to the
// (hopefully malloc'd at this point) flattenedClientTree. The void * is
// expected to point to an int.
void flattener(Client c, void *data) {
    int *index = (int *)data;
    flattenedClientTree[*index] = c;
    *index = *index + 1;
}

void initClients() {
    int *data = (int *) malloc(sizeof(int));
    *data = 0;
    walkTree(clientTree, clientCounter, data);
    numClients = *data;

    flattenedClientTree = (Client *) malloc(numClients * sizeof(Client *));
    *data = 0;
    walkTree(clientTree, flattener, data);
    free(data);
}

// Given an id and a client tree, returns if the tree is or has any children
// that are that id.
bool hasChild(uint8_t id, Client tree) {
    if(tree.id == id) {
        return true;
    }
    for(int i = 0; i < tree.numChildren; i++) {
        bool childHasChild = hasChild(id, tree.children[i]);
        if(childHasChild) {
            return true;
        }
    }
    return false;
}
