#include "tests.h"
#include "layout.h"
#include "request.h"
#include "path.h"

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
    Serial.println(F("Test failed!"));
    Serial.println(msg);
    return false;
}

///************************
// * Tests for client.cpp *
// ************************/
//
//bool testClientCounter() {
//    Serial.println(F("Running testClientCounter"));
//    int *data = (int *) malloc(sizeof(int));
//    *data = 0;
//    walkTree(testClient, clientCounter, data);
//    int numClients = *data;
//    free(data);
//
//    if(numClients != 5) {
//        Serial.print(F("number of clients: ")); Serial.println(numClients);
//        return error("Bad number of clients!");
//    }
//    Serial.println(F("testClientCounter succeeded"));
//    return true;
//}
//
//bool testFlattener() {
//    Serial.println(F("Running testClientCounter"));
//
//    int numClients = 5;
//    int *data = (int *) malloc(sizeof(int));
//    flattenedClientTree = (Client *) malloc(numClients * sizeof(Client *));
//    *data = 0;
//    walkTree(testClient, flattener, data);
//    free(data);
//
//    for(int i = 0; i < 5; i++) {
//        if(flattenedClientTree[i].id != i+2) {
//            return error("Flattened incorrectly!");
//        }
//    }
//    flattenedClientTree = NULL;
//    Serial.println(F("testFlattener succeeded"));
//    return true;
//}
//
//bool testHasChild() {
//    Serial.println(F("Running testHasChild"));
//
//    for(int i = 2; i < 7; i++) {
//        bool b = hasChild(i, testClient);
//        if(!b) {
//            return error("Bad value in loop");
//        }
//    }
//    if(hasChild(1, testClient)) {
//        return error("Bad value for id 1");
//    }
//    if(hasChild(7, testClient)) {
//        return error("Bad value for id 7");
//    }
//    Serial.println(F("testHasChild succeeded"));
//    return true;
//}
//
//bool runClientTests() {
//    Serial.println(F("Running tests for client.cpp"));
//    bool success = true;
//    success = success && testClientCounter();
//    success = success && testFlattener();
//    success = success && testHasChild();
//    return success;
//}

/*************************
 * Tests for request.cpp *
 *************************/

bool testAddRequest() {
    Serial.println(F("Running testAddRequest"));
    // Add one request
    uint8_t from = 3;
    uint8_t to = 6;
    addRequest(testClient, from, to);
    if(getNumRequests() != 1) {
        return error("Bad value for getNumRequests() #1!");
    }
    if(currentRequest()->from != from) {
        return error("Bad value for requests[0].from #1!");
    }
    if(currentRequest()->to != to) {
        return error("Bad value for requests[0].to #1!");
    }
    // Add another request
    from = 6;
    to = 3;
    addRequest(testClient, from, to);
    if(getNumRequests() != 2) {
        return error("Bad value for getNumRequests() #2!");
    }
    if((currentRequest()+1)->from != from) {
        return error("Bad value for requests[0].from #2!");
    }
    if((currentRequest()+1)->to != to) {
        return error("Bad value for requests[0].to #2!");
    }
    // Add a request with an existing from
    from = 6;
    to = 5;
    addRequest(testClient, from, to);
    if(getNumRequests() != 2) {
        return error("Bad value for getNumRequests() #3!");
    }
    Serial.println(F("testAddRequest succeeded"));
    resetRequests();
    return true;
}

//bool testFinishRequest() {
//    Serial.println(F("Running testFinishRequest"));
//    uint8_t from = 3;
//    uint8_t to = 6;
//    addRequest(testClient, to, from);
//    addRequest(testClient, from, to);
//    finishRequest();
//    if(getNumRequests() != 1) {
//        return error("Bad number of requests #1!");
//    }
//    if(currentRequest()->from != from) {
//        Serial.print(F("requests[0].from: ")); Serial.println(currentRequest()->from);
//        return error("Finished wrong request!");
//    }
//    finishRequest();
//    if(getNumRequests() != 0) {
//        return error("Bad number of requests #2!");
//    }
//    Serial.println(F("testFinishRequest succeeded"));
//    resetRequests();
//    return true;
//}

//bool testGetRequest() {
//    Serial.println(F("Running testGetRequest"));
//    uint8_t from = 3;
//    uint8_t to = 6;
//    addRequest(testClient, to, from);
//    addRequest(testClient, from, to);
//    Request *r = getRequest(from);
//    if(r->from != from || r->to != to) {
//        return error("getRequest returned wrong request!");
//    }
//    r = getRequest(64);
//    if(r != NULL) {
//        return error("getRequest should've returned NULL!");
//    }
//    Serial.println(F("testGetRequest succeeded"));
//    resetRequests();
//    return true;
//}

//bool testCancelRequest() {
//    Serial.println(F("Running testCancelRequest"));
//    uint8_t from = 3;
//    uint8_t to = 6;
//    addRequest(testClient, to, from);
//    addRequest(testClient, from, to);
//    cancelRequest(from);
//    if((currentRequest()+1)->state != Cancelled) {
//        return error("cancelRequest didn't cancel the request");
//    }
//    Serial.println(F("testCancelRequest succeeded"));
//    resetRequests();
//    return true;
//}

bool runRequestTests() {
    Serial.println(F("Running tests for request.cpp"));
    bool success = true;
    success = success && testAddRequest();
    //success = success && testFinishRequest();
    //success = success && testGetRequest();
    //success = success && testCancelRequest();
    return success;
}

/************************
 * Tests for client.cpp *
 ************************/

//bool testFindPath() {
//    Serial.println(F("Running testFindPath"));
//
//    Path p = findPath(testClient, 3);
//    if(p.numNodes != 1) {
//        return error("wrong number of nodes");
//    }
//    if(p.pathNodes[0].id != 2) {
//        return error("wrong path");
//    }
//    if(p.pathNodes[0].tube != 0) {
//        return error("wrong tube");
//    }
//
//    Serial.println(F("testFindPath succeeded"));
//    return true;
//}

//bool runPathTests() {
//    Serial.println(F("Running tests for path.cpp"));
//    return testFindPath();
//}


bool runTests() {
    Serial.println("hello world");
    Serial.println(F("-----------------"));
    Serial.println(F("Running all tests"));
    bool success = true;
    //success = success && runClientTests();
    success = success && runRequestTests();
    //success = success && runPathTests();
    return success;
}
