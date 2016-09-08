#include "request.h"

// The number of requests currently queued (this includes the active request),
// and the list of requests.
int numRequests = 0;
struct Request *requests = NULL;

// Adds a request with from as the starting terminal and to as the ending
// terminal. Does nothing if a request with a start of from is already queued.
void addRequest(uint8_t from, uint8_t to) {
    Request *existing = getRequest(from);
    if(existing != NULL) {
        // We already have a request in the queue from this guy!
        // Let's just ignore this.
        return;
    }
    Request r =
        {
            .from = from,
            .to = to,
            .fromPath = findPath(from),
            .toPath = findPath(to),
            .packetStarted = false,
            .packetFinished = false,
            .state = Queued
        };
    numRequests++;
    requests = (Request *)realloc(requests, numRequests);
    memcpy(&requests[numRequests-1], &r, numRequests * sizeof(struct Request));
}

// Removes the request from the front of the queue.
void finishRequest() {
    numRequests--;
    memcpy(requests, requests++, numRequests * sizeof(struct Request));
    requests = (Request *)realloc(requests, numRequests);
}

// Gets the request that has from as the starting point. Returns NULL if no
// such request exists.
struct Request *getRequest(uint8_t from) {
    for(int i = 0; i < numRequests; i++) {
        if(requests[i].from == from && requests[i].state != Cancelled) {
            return &requests[i];
        }
    }
    return NULL;
}

// Marks the request that has from as its starting point as cancelled.
void cancelRequest(uint8_t from) {
    for(int i = 0; i < numRequests; i++) {
        if(requests[i].from == from) {
            requests[i].state = Cancelled;
        }
    }
}

struct Request *currentRequest() {
    return &requests[0];
}
