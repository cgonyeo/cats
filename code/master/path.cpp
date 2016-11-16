#include "path.h"

void findPathHelper(uint8_t id, const Client *tree, struct Path *p) {
    if(tree->id == id) {
        return;
    }
    for(int i = 0; i < tree->numChildren; i++) {
        if(hasChild(id, &(tree->children[i]))) {
            p->numNodes++;
            p->pathNodes = (struct PathNode *)realloc(p->pathNodes,
                                     p->numNodes * sizeof(struct PathNode));
            if(p->pathNodes == NULL) {
                Serial.println(F("holy shitfuck"));
            }
            Serial.print(F("Adding id ")); Serial.println(tree->id);
            p->pathNodes[p->numNodes-1] = {
                .id = tree->id,
                .tube = i,
                .configured = false,
                .packetReached = false
            };
            Serial.print(F("Added id ")); Serial.println(p->pathNodes[p->numNodes-1].id);
            findPathHelper(id, &(tree->children[i]), p);
        }
    }
}

// findPath will find a Path from the root of the clientTree to the given id.
struct Path *findPath(const Client *client, uint8_t id) {
    Serial.print(F("Finding path to ")); Serial.println(id);
    struct Path *p = (struct Path *)malloc(sizeof(struct Path));
    if(p == NULL) {
        Serial.println(F("holy shitfuck"));
    }
    p->numNodes = 0;
    findPathHelper(id, client, p);
    Serial.print(F("MALLOC: path ")); Serial.println((int)p);
    return p;
}

void freePath(struct Path *p) {
    if(p->numNodes != 0) {
        free(p->pathNodes);
    }
    Serial.print(F("FREE: path ")); Serial.println((int)p);
    free(p);
}
