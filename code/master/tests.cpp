#include "tests.h"
#include "layout.h"
#include "request.h"

Client testClient = 
    {
        .id = 2,
        .type = Router,
        .label = "root router",
        .numChildren = 2,
        .children = (Client[]) {
            {
                .id = 3,
                .type = Terminal,
                .label = "terminal 1",
                .numChildren = 0,
                .children = NULL
            },
            {
                .id = 4,
                .type = Terminal,
                .label = "router 2",
                .numChildren = 2,
                .children = (Client[]) {
                    {
                        .id = 5,
                        .type = Terminal,
                        .label = "terminal 2",
                        .numChildren = 0,
                        .children = NULL
                    },
                    {
                        .id = 6,
                        .type = Terminal,
                        .label = "terminal 3",
                        .numChildren = 0,
                        .children = NULL
                    }
                }
            }
        }
    };

/*******************
 * Testing Helpers *
 *******************/

bool error(const char *msg) {
    Serial.println("Test failed!");
    Serial.println(msg);
    return false;
}

/************************
 * Tests for client.cpp *
 ************************/

bool testClientCounter() {
    Serial.println("Running testClientCounter");

    int *data = (int *) malloc(sizeof(int));
    *data = 0;
    walkTree(clientTree, clientCounter, data);
    int numClients = *data;
    free(data);

    if(numClients != 5) {
        return error("Bad number of clients!");
    }
    Serial.println("testClientCounter succeeded");
    return true;
}

bool testFlattener() {
    Serial.println("Running testClientCounter");

    int numClients = 5;
    int *data = (int *) malloc(sizeof(int));
    flattenedClientTree = (Client *) malloc(numClients * sizeof(Client *));
    *data = 0;
    walkTree(clientTree, flattener, data);
    free(data);

    for(int i = 0; i < 5; i++) {
        if(flattenedClientTree[i].id != i+2) {
            return error("Flattened incorrectly!");
        }
    }
    flattenedClientTree = NULL;
    Serial.println("testFlattener succeeded");
    return true;
}

bool testHasChild() {
    Serial.println("Running testHasChild");

    for(int i = 2; i < 7; i++) {
        bool b = hasChild(i, testClient);
        if(!b) {
            return error("Bad value in loop");
        }
    }
    if(hasChild(1, testClient)) {
        return error("Bad value for id 1");
    }
    if(hasChild(7, testClient)) {
        return error("Bad value for id 7");
    }
    Serial.println("testHasChild succeeded");
    return true;
}

bool runClientTests() {
    Serial.println("Running tests for client.cpp");
    bool success = true;
    success = success && testClientCounter();
    success = success && testFlattener();
    success = success && testHasChild();
    return success;
}

/*************************
 * Tests for request.cpp *
 *************************/

void resetRequests() {
    numRequests = 0;
    // Meh
    //if(requests != NULL) {
    //    free(requests);
    //}
    requests = NULL;
}

bool testAddRequest() {
    Serial.println("Running testAddRequest");
    // Add one request
    uint8_t from = 3;
    uint8_t to = 6;
    addRequest(testClient, from, to);
    if(numRequests != 1) {
        return error("Bad value for numRequests #1!");
    }
    if(requests[0].from != from) {
        return error("Bad value for requests[0].from #1!");
    }
    if(requests[0].to != to) {
        return error("Bad value for requests[0].to #1!");
    }
    // Add another request
    from = 6;
    to = 3;
    addRequest(testClient, from, to);
    if(numRequests != 2) {
        return error("Bad value for numRequests #2!");
    }
    if(requests[1].from != from) {
        return error("Bad value for requests[0].from #2!");
    }
    if(requests[1].to != to) {
        return error("Bad value for requests[0].to #2!");
    }
    // Add a request with an existing from
    from = 6;
    to = 5;
    addRequest(testClient, from, to);
    if(numRequests != 2) {
        return error("Bad value for numRequests #3!");
    }
    Serial.println("testAddRequest succeeded");
    resetRequests();
    return true;
}

bool testFinishRequest() {
    Serial.println("Running testFinishRequest");
    uint8_t from = 3;
    uint8_t to = 6;
    addRequest(testClient, to, from);
    addRequest(testClient, from, to);
    finishRequest();
    if(numRequests != 1) {
        return error("Bad number of requests #1!");
    }
    if(requests[0].from != from) {
        return error("Finished wrong request!");
    }
    finishRequest();
    if(numRequests != 0) {
        return error("Bad number of requests #2!");
    }
    Serial.println("testFinishRequest succeeded");
    resetRequests();
    return true;
}

bool testGetRequest() {
    Serial.println("Running testGetRequest");
    uint8_t from = 3;
    uint8_t to = 6;
    addRequest(testClient, to, from);
    addRequest(testClient, from, to);
    Request *r = getRequest(from);
    if(r->from != from || r->to != to) {
        return error("getRequest returned wrong request!");
    }
    r = getRequest(64);
    if(r != NULL) {
        return error("getRequest should've returned NULL!");
    }
    Serial.println("testGetRequest succeeded");
    resetRequests();
    return true;
}

bool testCancelRequest() {
    Serial.println("Running testCancelRequest");
    uint8_t from = 3;
    uint8_t to = 6;
    addRequest(testClient, to, from);
    addRequest(testClient, from, to);
    cancelRequest(from);
    if(requests[1].state != Cancelled) {
        return error("cancelRequest didn't cancel the request");
    }
    Serial.println("testCancelRequest succeeded");
    resetRequests();
    return true;
}

bool runRequestTests() {
    Serial.println("Running tests for request.cpp");
    bool success = true;
    success = success && testAddRequest();
    success = success && testFinishRequest();
    success = success && testGetRequest();
    success = success && testCancelRequest();
    return success;
}




bool runTests() {
    bool success = true;
    success = success && runClientTests();
    success = success && runRequestTests();
    return success;
}
