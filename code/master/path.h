#ifndef PATH
#define PATH

#include "shared/shared.h"
#include "client.h"

// A Path (which includes PathNodes) represents the path a capsule will follow
// through the system. Each request results in two paths: one from a terminal
// to the root of the tree, and one from the root of the tree to a different
// terminal.
//
// Each PathNode includes fields to track the progress of the request wrt that
// node.
struct PathNode {
    uint8_t id;            // The id of a router in the system
    int     tube;          // The tube this router should be configured to use
    bool    configured;    // Whether or not this router has finished
                           // configuration
    bool    packetReached; // Whether or not the capsule has reached this router
};

struct Path {
    int              numNodes;  // The number of PathNodes in this path
    struct PathNode *pathNodes; // The nodes of this path
};

struct Path findPath(Client client, uint8_t id);

#endif
