#ifndef LAYOUT
#define LAYOUT

#include "shared/shared.h"

// A client in the system is either a Terminal or a Router. Well, or a vacuum,
// but that's handled differently.
typedef enum ClientType {
    Terminal,
    Router
} ClientType;

// A Client represents a client in this system. Clients are pinged by the
// master for updates, and clients are sent instructions by the master they
// must carry out (like changing tubes or alerting users).
struct Client {
    uint8_t     id;          // The id of this client
    ClientType  type;        // The type of this client
    const char *label;       // A human-readable label for this client
    int         numChildren; // The number of children this client has
    Client     *children;    // This client's children
    bool        hasError;    // Whether or not this client is suffering an error
                             // that prevents operation.
    bool        isUnreachable; // Whether or not this client is unreachable.
};

extern Client clientTree;

#endif
