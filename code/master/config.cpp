#include "config.h"

// Marks the router with the given id as having finished its configuration for
// the current request. Will select the fromPath or the toPath depending on the
// state of the request.
void markConfigComplete(uint8_t id) {
    Request *r = currentRequest();
    struct Path p;
    if(r->state == ConfigForPull) {
        p = r->fromPath;
    } else {
        p = r->toPath;
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
    if(getNumRequests() == 0) {
        // There are no requests?!?
        return;
    }
    Request *r = currentRequest();
    ClientType type = (ClientType)0xFF;
    for(int i = 0; i < numClients; i++) {
        if(flattenedClientTree[i].id == id) {
            type = flattenedClientTree[i].type;
        }
    }
    switch(type) {
        case Router: {
            struct Path p;
            if(r->state == Pulling) {
                p = r->fromPath;
            } else {
                p = r->toPath;
            }
            for(int i = 0; i < p.numNodes; i++) {
                if(p.pathNodes[i].id == id) {
                    p.pathNodes[i].packetReached = true;
                }
            }
            break;
        }
        case Terminal:
            if(r->state == Pulling) {
                if(r->from == id) {
                    r->packetStarted = true;
                } else {
                    // TODO uhh, started from wrong terminal? maybe ignore it?
                }
            } else {
                if(r->to == id) {
                    r->packetFinished = true;
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
                .id = MASTER_ID,
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
void configVacuum(VacuumState state) {
    uint8_t msg;
    switch(state) {
        case VPulling:
            msg = CHANGE_TO_TUBE_0;
            break;
        case VPushing:
            msg = CHANGE_TO_TUBE_1;
            break;
    }
    writeMsgAndExpectAck(
        {
            .id = MASTER_ID,
            .message = msg,
            .data = VACUUM_ID
        }
    );
}

// Instructs the vacuum to start.
void startVacuum() {
    writeMsgAndExpectAck(
        {
            .id = MASTER_ID,
            .message = VACUUM_ON,
            .data = VACUUM_ID
        }
    );
}

// Instructs the vacuum to stop.
void stopVacuum() {
    writeMsgAndExpectAck(
        {
            .id = MASTER_ID,
            .message = VACUUM_OFF,
            .data = VACUUM_ID
        }
    );
}
