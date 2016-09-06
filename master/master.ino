#include <SoftwareSerial.h>

// The ID of this arduino
#define MY_ID 0

// The ID of the vacuum
#define VACUUM_ID 1

// Stuff for the RS485 communications
#define SSerialRX        10  //Serial Receive pin
#define SSerialTX        11  //Serial Transmit pin
#define SSerialTxControl 3   //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW
#define Pin13LED         13
SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

// Master -> Client Messages
#define STATUS_UPDATE    0x00
#define CHANGE_TO_TUBE_0 0x01
#define CHANGE_TO_TUBE_1 0x02
#define CHANGE_TO_TUBE_2 0x03
#define CHANGE_TO_TUBE_3 0x04
#define INCOMING_CAPSULE 0x05
#define CLEAR_INCOMING   0x06
#define SYSTEM_OFFLINE   0x07
#define CLEAR_OFFLINE    0x08
#define SYSTEM_ERROR     0x09
#define CLEAR_ERROR      0x0a
#define READY_TO_PULL    0x0b
#define CLEAR_READY      0x0c
#define REQUEST_QUEUED   0x0d
#define CLEAR_QUEUED     0x0e
#define VACUUM_ON        0x0f
#define VACUUM_OFF       0x10

// Client -> Mater Messages
#define NOTHING                0x00
#define ACKNOWLEDGED           0x01
#define ITEM_DETECTED          0x02
#define CONFIGURATION_COMPLETE 0x03
#define ERROR                  0x04
#define NEW_REQUEST            0x05
#define CANCEL_REQUEST         0x06

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
};

// The clientTree is the tree of all clients available in the system. It is
// used by the master as the list of clients to ping for updates, and the
// structure of the tree is used to determine routing paths.
//
// Not included in the tree is the vacuum node, which is treated somewhat
// differently during routing decisions.
//
// On program startup the flattenedClientTree variable is created to point to a
// flattened version of this tree, which is easier to iterate over.
//
// All leaf nodes must be of type Terminal, and all non-leaf nodes must be of
// type router.
//
// Care must be taken to ensure that for each node, numChildren is accurate. If
// it's too low parts of the tree will be ignored during routing decisions, and
// if it's too high undefined behavior will creep in.
//
// The label field is simply used to improve the human readability of log
// messages.
//
// Each id _must_ be unique! Ids 0 and 1 are reserved for the master and the
// vacuum.
const Client clientTree =
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
                .numChildren = 1,
                .children = (Client[]) {
                    {
                        .id = 5,
                        .type = Terminal,
                        .label = "terminal 2",
                        .numChildren = 0,
                        .children = NULL
                    }
                }
            }
        }
    };

// flattenedClientTree and numClients are both generated during program setup
// from the clientTree. The names are hopefully self-explanatory.
Client *flattenedClientTree;
int numClients;

// walkTree will walk the specified client tree, calling the provided function
// on each client, and passing in the provided void * each call.
void walkTree(Client tree, void (*walkFunc) (Client, void *), void *data) {
    walkFunc(tree, data);
    for(int i = 0; i < tree.numChildren; i++) {
        walkTree(tree.children[i], walkFunc, data);
    }
}

// clientCounter, when provided to walkTree, will count up the number of
// clients. The void * is expected to point to an int.
void clientCounter(Client c, void *data) {
    int *num = (int *)data;
    *num = *num + 1;
};

// flattener will, when provided to walkTree, will add each client to the
// (hopefully malloc'd at this point) flattenedClientTree. The void * is
// expected to point to an int.
void flattener(Client c, void *data) {
    int *index = (int *)data;
    flattenedClientTree[*index] = c;
    *index = *index + 1;
}

// A Message represents a message sent over the wire.
struct Message {
    uint8_t id;      // The sender of the message
    uint8_t message; // The message type (see #define's earlier in this file)
    uint8_t data;    // An arbitrary byte of data, meaning dependent on type
};

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

// Given an id and a client tree, returns if the tree is or has any children
// that are that id.
bool hasChild(uint8_t id, Client tree) {
    if(tree.id == id) {
        return true;
    }
    for(int i = 0; i < tree.numChildren; i++) {
        bool childHasChild = hasChild(id, tree.children[i]);
        if(childHasChild) {
            return true;
        }
    }
    return false;
}

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

// The number of requests currently queued (this includes the active request),
// and the list of requests.
int numRequests = 0;
struct Request *requests = NULL;

// Adds a request with from as the starting terminal and to as the ending
// terminal. Does nothing if a request with a start of from is already queued.
//
// TODO: do nothing if a request with a start of from is already queued
void addRequest(uint8_t from, uint8_t to) {
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

// Gets the request that has from as the starting point
struct Request getRequest(uint8_t from) {
    for(int i = 0; i < numRequests; i++) {
        if(requests[i].from == from) {
            return requests[i];
        }
    }
    // TODO: error
    return requests[0];
}

// Marks the request that has from as its starting point as cancelled.
void cancelRequest(uint8_t from) {
    for(int i = 0; i < numRequests; i++) {
        if(requests[i].from == from) {
            requests[i].state = Cancelled;
        }
    }
}

// Marks the router with the given id as having finished its configuration for
// the current request. Will select the fromPath or the toPath depending on the
// state of the request.
void markConfigComplete(uint8_t id) {
    Request r = requests[0];
    struct Path p;
    if(r.state == ConfigForPull) {
        p = r.fromPath;
    } else {
        p = r.toPath;
    }
    for(int i = 0; i < p.numNodes; i++) {
        if(p.pathNodes[i].id == id) {
            p.pathNodes[i].configured = true;
        }
    }
}

// A client has detected a packet! This function marks the packet having
// reached the client with the given id in the current request.
void markPacketReached(uint8_t id) {
    if(numRequests == 0) {
        // There are no requests?!?
        return;
    }
    ClientType type = (ClientType)0xFF;
    for(int i = 0; i < numClients; i++) {
        if(flattenedClientTree[i].id == id) {
            type = flattenedClientTree[i].type;
        }
    }
    switch(type) {
        case Router: {
            struct Path p;
            if(requests[0].state == Pulling) {
                p = requests[0].fromPath;
            } else {
                p = requests[0].toPath;
            }
            for(int i = 0; i < p.numNodes; i++) {
                if(p.pathNodes[i].id == id) {
                    p.pathNodes[i].packetReached = true;
                }
            }
            break;
        }
        case Terminal:
            if(requests[0].state == Pulling) {
                if(requests[0].from == id) {
                    requests[0].packetStarted = true;
                } else {
                    // TODO uhh, started from wrong terminal? maybe ignore it?
                }
            } else {
                if(requests[0].to == id) {
                    requests[0].packetFinished = true;
                } else {
                    // TODO uhh, finished at wrong terminal? maybe ignore it?
                }
            }
            break;
        default:
            // TODO CLIENT NOT FOUND!
            break;
    }
}

// Given a Path, this function will send instructions to each router along the
// path to configure themselves to the correct tube.
void configureForPath(struct Path p) {
    for(int i = 0; i < p.numNodes; i++) {
        uint8_t msgToSend = 0xFF;
        switch(p.pathNodes[i].tube) {
            case 0: msgToSend = CHANGE_TO_TUBE_0;
            case 1: msgToSend = CHANGE_TO_TUBE_1;
            case 2: msgToSend = CHANGE_TO_TUBE_2;
            case 3: msgToSend = CHANGE_TO_TUBE_3;
        }
        writeMsgAndExpectAck(
            {
                .id = MY_ID,
                .message = msgToSend,
                .data = p.pathNodes[i].id
            }
        );
    }
}

// This function will determine whether or not the given path has completed its
// configuration.
bool pathIsConfigured(struct Path p) {
    bool allConfigured = true;
    for(int i = 0; i < p.numNodes; i++) {
        allConfigured = allConfigured && p.pathNodes[i].configured;
    }
    return allConfigured;
}

// Configures the vacuum to push or pull.
void configVacuum(bool pull) {
    uint8_t msg;
    if(pull) {
        msg = CHANGE_TO_TUBE_1;
    } else {
        msg = CHANGE_TO_TUBE_3;
    }
    writeMsgAndExpectAck(
        {
            .id = MY_ID,
            .message = msg,
            .data = VACUUM_ID
        }
    );
}

// Instructs the vacuum to start.
void startVacuum() {
    writeMsgAndExpectAck(
        {
            .id = MY_ID,
            .message = VACUUM_ON,
            .data = VACUUM_ID
        }
    );
}

// Instructs the vacuum to stop.
void stopVacuum() {
    writeMsgAndExpectAck(
        {
            .id = MY_ID,
            .message = VACUUM_OFF,
            .data = VACUUM_ID
        }
    );
}

// Writes out the given message to the network, and then waits for and returns
// the reply.
struct Message writeMsg(struct Message msg) {
    digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit   
    RS485Serial.write(msg.id);
    RS485Serial.write(msg.message);
    RS485Serial.write(msg.data);
    digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit       

    int bytesReceived = 0;
    Message receivedMsg;
    while(true) {
        if(RS485Serial.available()) {
            uint8_t data = RS485Serial.read();
            switch(bytesReceived) {
                case 0:
                    if(data != msg.data) {
                        // TODO WOAH WTF
                        // got reply from wrong client?!?!
                    }
                    receivedMsg.id = data;
                    break;
                case 1:
                    receivedMsg.message = data;
                    break;
                case 3:
                    receivedMsg.data = data;
                    return receivedMsg;
            };
            bytesReceived++;
        }
    }
}

// Writes out the given message to the network, waits for the reply, checks
// that the reply was an ACKNOWLEDGE, and doesn't return the reply.
void writeMsgAndExpectAck(struct Message msg) {
    Message reply = writeMsg(msg);
    if(reply.message != ACKNOWLEDGED) {
        // TODO WOAH WTF
    }
}

// setup, as per arduino tooling, runs once when the arduino starts.
void setup()
{
    Serial.begin(9600);

    int *data = (int *) malloc(sizeof(int));
    *data = 0;
    walkTree(clientTree, clientCounter, data);
    numClients = *data;

    flattenedClientTree = (Client *) malloc(numClients * sizeof(Client *));
    *data = 0;
    walkTree(clientTree, flattener, data);
    free(data);

    Serial.print("System is configured for ");
    Serial.print(numClients);
    Serial.println(" clients.");

    Serial.println("These are their names:");
    for(int i = 0; i < numClients; i++) {
        Serial.print(" - ");
        Serial.println(flattenedClientTree[i].label);
    }


    Serial.println("here's the path to terminal with id 4:");
    struct Path p = findPath(4);
    for(int i = 0; i < p.numNodes; i++) {
        Serial.print(" - id:");
        Serial.print(p.pathNodes[i].id);
        Serial.print(" tube:");
        Serial.println(p.pathNodes[i].tube);
    }




    pinMode(Pin13LED, OUTPUT);   
    pinMode(SSerialTxControl, OUTPUT);    

    digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver   

    // Start the software serial port, to another device
    RS485Serial.begin(4800);   // set the data rate 
}

// loop, as per arduino tooling, runs repeatedly after setup is complete.
void loop()
{
    // Iterate over each client, asking it for a status update.
    for(int i = 0; i < numClients; i++) {
        // Ask the next guy for their status
        Message resp = writeMsg(
            {
                .id = MY_ID,
                .message = STATUS_UPDATE,
                .data = (uint8_t)flattenedClientTree[i].id
            }
        );

        switch(resp.message) {
            case NOTHING:
                // This client has nothing! Therefore we do nothing!
                break;
            case ACKNOWLEDGED:
                // TODO but we didn't tell them anything...
                break;
            case ITEM_DETECTED:
                // This client has detected a capsule move by it
                markPacketReached(resp.id);
                break;
            case CONFIGURATION_COMPLETE:
                // This client (presumably a router) has finished the
                // configuration we requested.
                markConfigComplete(resp.id);
                break;
            case ERROR:
                // TODO woah wtf
                break;
            case NEW_REQUEST: {
                // This client (presumably a terminal) has a new request from a
                // user. We should record it, tell the source that we have
                // received the request, and the destination that they will
                // have an incoming capsule.
                writeMsgAndExpectAck(
                    {
                        .id = MY_ID,
                        .message = REQUEST_QUEUED,
                        .data = resp.id
                    }
                );
                writeMsgAndExpectAck(
                    {
                        .id = MY_ID,
                        .message = INCOMING_CAPSULE,
                        .data = resp.data
                    }
                );
                addRequest(resp.id, resp.data);
                break;
            }
            case CANCEL_REQUEST: {
                // This client (presumably a terminal) has been instructed by a
                // user to cancel its request. Find the request, and cancel it
                // if the capsule hasn't entered the system yet. Once the
                // capsule has entered the system, the request is no longer
                // cancellable.
                Request requestToCancel = getRequest(resp.id); // TODO: handle not existing
                if(requestToCancel.state == Queued ||
                   requestToCancel.state == ConfigForPull ||
                   requestToCancel.state == ReadyToPull) {
                    writeMsgAndExpectAck(
                        {
                            .id = MY_ID,
                            .message = CLEAR_QUEUED,
                            .data = resp.id
                        }
                    );
                    writeMsgAndExpectAck(
                        {
                            .id = MY_ID,
                            .message = CLEAR_READY,
                            .data = resp.id
                        }
                    );
                    writeMsgAndExpectAck(
                        {
                            .id = MY_ID,
                            .message = CLEAR_INCOMING,
                            .data = requestToCancel.to
                        }
                    );
                    cancelRequest(resp.id);
                }
                break;
            }
            default:
                // TODO WOAH WTF
                // unexpected message
                break;
        }
    }

    // Ok, we've asked all the clients for their updates. If there are any
    // active requests, let's see if we need to do anything for it.
    if(numRequests > 0) {
        Request curr = requests[0];
        switch(curr.state) {
            case Queued:
                // The top request is queued, let's send out configuration
                // instructions.
                configureForPath(curr.fromPath);
                configVacuum(true);
                curr.state = ConfigForPull;
                break;
            case ConfigForPull: {
                // The top request is being configured for pulling. Let's check
                // if it's done, and start pulling if it is.
                if(pathIsConfigured(curr.fromPath)) {
                    // TODO check if vacuum is configured
                    startVacuum();
                    writeMsgAndExpectAck(
                        {
                            .id = MY_ID,
                            .message = READY_TO_PULL,
                            .data = curr.from
                        }
                    );
                    curr.state = ReadyToPull;
                }
                break;
            }
            case ReadyToPull:
                // The top request is pulling, but the capsule hasn't entered
                // the system yet. Let's check for the capsule, and make this
                // request non-cancellable if we've seen it.
                if(curr.packetStarted) {
                    curr.state = Pulling;
                }
                break;
            case Pulling:
                // The top request is pulling. Check if the capsule has reached
                // the root of the system, and if it has stop the vacuum and
                // configure the system for pushing.
                if(curr.fromPath.pathNodes[0].packetReached) {
                    stopVacuum();
                    configureForPath(curr.toPath);
                    configVacuum(false);
                    curr.state = ConfigForPush;
                }
                break;
            case ConfigForPush:
                // The top request is being configured for pushing. Let's check
                // if it's done, and start pushing if it is.
                if(pathIsConfigured(curr.toPath)) {
                    // TODO check if vacuum is configured
                    startVacuum();
                    curr.state = Pushing;
                }
                break;
            case Pushing:
                // The system is pushing. Let's check if the capsule has
                // reached its destination, and stop pushing if it has.
                if(curr.packetFinished) {
                    stopVacuum();
                    curr.state = Finished;
                }
                break;
            case Cancelled:
                // This request has been cancelled! We should tell the vacuum
                // to stop (in case it's running).
                 stopVacuum();
            case Finished:
                // This request has been cancelled or finished properly. Let's
                // remove it from the queue.
                finishRequest();
                break;
            default:
                // TODO WOAH WTF
                // unexpected message
                break;
        };
    }


    //digitalWrite(Pin13LED, HIGH);  // Show activity
    //if (Serial.available())
    //{
    //    byteReceived = Serial.read();

    //    digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit   
    //    RS485Serial.write(byteReceived);          // Send byte to Remote Arduino

    //    digitalWrite(Pin13LED, LOW);  // Show activity    
    //    delay(10);
    //    digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit       
    //}

    //if (RS485Serial.available())  //Look for data from other Arduino
    //{
    //    digitalWrite(Pin13LED, HIGH);  // Show activity
    //    byteReceived = RS485Serial.read();    // Read received byte
    //    Serial.write(byteReceived);        // Show on Serial Monitor
    //    delay(10);
    //    digitalWrite(Pin13LED, LOW);  // Show activity   
    //}  

}
