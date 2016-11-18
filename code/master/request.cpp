#include "request.h"
#include "path.h"

#define maxRequests 4

struct Request requestStart[maxRequests];

int requestHead = 0;
int numRequests = 0;

// Adds a request with from as the starting terminal and to as the ending
// terminal. Does nothing if a request with a start of from is already queued.
bool addRequest(const Client *client, uint8_t from, uint8_t to) {
    if(numRequests > maxRequests) {
        return false;
    }
    Request *existing = getRequest(from);
    if(existing != NULL) {
        // We already have a request in the queue from this guy!
        // Let's just ignore this.
        Serial.println(F("Received a duplicate request. Ignoring."));
        return false;
    }
    int requestLoc = (requestHead + numRequests) % maxRequests;
    requestStart[requestLoc] =
        {
            .from = from,
            .to = to,
            .fromPath = findPath(client, from),
            .toPath = findPath(client, to),
            .packetStarted = false,
            .packetFinished = false,
            .state = Queued
        };
    numRequests++;
    return true;
}

// Removes the request from the front of the queue.
void finishRequest() {
    if(numRequests == 0) {
        // There's no requests to finish?
        Serial.println(F("Called finishRequest with empty request queue"));
        return;
    }
    freePath((requestStart+requestHead)->fromPath);
    freePath((requestStart+requestHead)->toPath);
    requestHead = (requestHead + 1) % maxRequests;
    numRequests--;
    if((requestStart + requestHead)->state == Cancelled) {
        // We just advanced to a cancelled request. Let's remove it too.
        Serial.println(F("next one is cancelled, nuke it"));
        finishRequest();
    }
}

// Gets the request that has from as the starting point. Returns NULL if no
// such request exists.
struct Request *getRequest(uint8_t from) {
    int i = requestHead;
    while(i != (requestHead + numRequests) % maxRequests) {
        struct Request *r = requestStart + i;
        if(r->from == from && r->state != Cancelled) {
            return r;
        }
        i = (i + 1) % maxRequests;
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
    while(numRequests > 0) {
        finishRequest();
    }
    requestHead = 0;
}

void printRequests() {
    if(numRequests == 0) {
        Serial.println(F("Request queue is empty"));
        return;
    }
    int i = requestHead;
    while(i != (requestHead + numRequests) % maxRequests) {
        struct Request *r = requestStart + i;
        Serial.print(F("Request ")); Serial.println(i);
        Serial.print(F("	From:	")); Serial.println(r->from);
        Serial.print(F("	To:	")); Serial.println(r->to);
        i = (i + 1) % maxRequests;
    }
}
