#include "path.h"

struct Path findPathHelper(uint8_t id, Client tree, struct Path p) {
    if(tree.id == id) {
        return p;
    }
    for(int i = 0; i < tree.numChildren; i++) {
        if(hasChild(id, tree.children[i])) {
            p.numNodes++;
            p.pathNodes = (struct PathNode *)realloc(p.pathNodes,
                                     p.numNodes * sizeof(struct PathNode));
            p.pathNodes[p.numNodes-1] = {
                .id = tree.id,
                .tube = i,
                .configured = false,
                .packetReached = false
            };
            return findPathHelper(id, tree.children[i], p);
        }
    }
}

// findPath will find a Path from the root of the clientTree to the given id.
struct Path findPath(uint8_t id) {
    return findPathHelper(id, clientTree, {0, NULL});
}
