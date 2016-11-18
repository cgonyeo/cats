#include "path.h"
#include "MemoryFree.h"

void print2(int i) {
    Serial.print(F("Terminal 2 name "));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.println(flattenedClientTree[1].label);
    Serial.print(F("We currently have this much free memory: "));
    Serial.println(freeMemory());
}

void findPathHelper(uint8_t id, const Client *tree, struct Path *p) {
    if(tree->id == id) {
        return;
    }
    for(int i = 0; i < tree->numChildren; i++) {
        if(hasChild(id, tree->children + i)) {
            print2(2);
            p->numNodes++;
            print2(3);
            p->pathNodes = (struct PathNode *)realloc(p->pathNodes,
                                     p->numNodes * sizeof(struct PathNode));
            print2(4);
            if(p->pathNodes == NULL) {
                Serial.println(F("holy shitfuck"));
            }
            print2(5);
            memset(p->pathNodes + (p->numNodes-1), 0, sizeof(struct PathNode));
            p->pathNodes[p->numNodes-1].id = tree->id;
            p->pathNodes[p->numNodes-1].tube = i;
            p->pathNodes[p->numNodes-1].configured = false;
            p->pathNodes[p->numNodes-1].packetReached = false;
            print2(6);
            findPathHelper(id, tree->children + i, p);
        }
    }
}

// findPath will find a Path from the root of the clientTree to the given id.
struct Path *findPath(const Client *client, uint8_t id) {
    struct Path *p = (struct Path *)malloc(sizeof(struct Path));
    if(p == NULL) {
        Serial.println(F("holy shitfuck"));
    }
    p->numNodes = 0;
    p->pathNodes = NULL;
    print2(0);
    findPathHelper(id, client, p);
    print2(100);
    return p;
}

void freePath(struct Path *p) {
    if(p->numNodes != 0) {
        free(p->pathNodes);
    }
    free(p);
}
