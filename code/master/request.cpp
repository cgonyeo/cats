#include "request.h"
#include "path.h"

struct Request *requestStart = NULL;

int requestHead = 0;
int maxRequests = 4;
int numRequests = 0;

// Initializes the requests circular queue
void initRequestQueue() {
    requestStart = (struct Request *)malloc(maxRequests * sizeof(struct Request));
}

// Adds a request with from as the starting terminal and to as the ending
// terminal. Does nothing if a request with a start of from is already queued.
void addRequest(Client client, uint8_t from, uint8_t to) {
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
            .fromPath = findPath(client, from),
            .toPath = findPath(client, to),
            .packetStarted = false,
            .packetFinished = false,
            .state = Queued
        };
    int requestLoc = (requestHead + numRequests) % maxRequests;
    numRequests++;
    memcpy((requestStart + requestLoc), &r, sizeof(struct Request));

    Serial.println("fuck");
    //Serial.print("Just added request to loc: "); Serial.println(requestLoc);
    //Serial.print("New numRequests: "); Serial.println(numRequests);
    //Serial.print("New request was from: "); Serial.println((requestStart+requestLoc)->from);
}

// Removes the request from the front of the queue.
void finishRequest() {
    requestHead = (requestHead + 1) % maxRequests;
    numRequests--;
    if((requestStart + requestHead)->state == Cancelled) {
        // We just advanced to a cancelled request. Let's remove it too.
        finishRequest();
    }
}

// Gets the request that has from as the starting point. Returns NULL if no
// such request exists.
struct Request *getRequest(uint8_t from) {
    int i = requestHead;
    while(i != (requestHead + numRequests) % maxRequests) {
        i = (i + 1) % maxRequests;
        struct Request *r = requestStart + i;
        if(r->from == from && r->state != Cancelled) {
            return r;
        }
    }
    return NULL;
}

// Marks the request that has from as its starting point as cancelled.
void cancelRequest(uint8_t from) {
    int i = requestHead;
    while(i != (requestHead + numRequests) % maxRequests) {
        i = (i + 1) % maxRequests;
        struct Request *r = requestStart + i;
        if(r->from == from && r->state != Cancelled) {
            r->state = Cancelled;
        }
    }
}

struct Request *currentRequest() {
    return requestStart + requestHead;
}

int getNumRequests() {
    return numRequests;
}

void resetRequests() {
    requestHead = 0;
    numRequests = 0;
}
