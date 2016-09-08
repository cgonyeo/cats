#ifndef REQUEST
#define REQUEST

#include "shared/shared.h"
#include "path.h"

// All the different states a request can be in
typedef enum RequestState {
    Queued,         // The request has been acknowledged by the system, and is
                    // in the queue to be handled.

    ConfigForPull,  // The system has begun configuring routers for pulling the
                    // capsule to the root of the tree.

    ReadyToPull,    // The system has finished configuration, started the
                    // vacuum, and is waiting for the capsule to be placed in
                    // the terminal.

    Pulling,        // The system has detected the capsule enter the system at
                    // the endpoint, and the capsule is on its way to the root
                    // of the system.

    ConfigForPush,  // The capsule has reached the root of the system, and the
                    // vacuum has been disabled. The system has now begun
                    // configuring routers for pushing the capsule to the
                    // destination terminal.

    Pushing,        // The system has finished configuration, started the
                    // vacuum, and is waiting for the capsule to be detected at
                    // the target destination.

    Finished,       // The capsule has reached its destination, the vacuum is
                    // disabled, and the request is finished.

    Cancelled       // The request has been cancelled by the user.
} RequestState;

// A request is a thing received from a terminal, and it represents a request
// from a user to send a capsule through the system to a destination.
//
// The most basic form of a request is just the start and destination of a
// capsule (from and to), but this struct includes other fields for tracking
// the state of the request.
struct Request {
    uint8_t from;         // The ID of the terminal the capsule will come from

    uint8_t to;           // The ID of the terminal the capsule will go to

    struct Path fromPath; // The path the capsule will follow from the start to
                          // the root of the system

    struct Path toPath;   // The path the capsule will follow from the root of
                          // the system to the destination

    bool packetStarted;   // Whether or not the packet has started its journey

    bool packetFinished;  // Whether or not the packet has reached its
                          // destination

    RequestState state;   // The current state of this request
};

void addRequest(uint8_t from, uint8_t to);
void finishRequest();
struct Request *getRequest(uint8_t from);
void cancelRequest(uint8_t from);
struct Request *currentRequest();

extern int numRequests;

#endif
