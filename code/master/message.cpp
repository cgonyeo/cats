#include "message.h"

// Writes out the given message to the network, and then waits for and returns
// the reply.
Message writeMsgMaster(Message msg) {
    writeMsg(msg);
    Message resp = getMsg();
    if(resp.id != msg.data) {
        // TODO WOAH WTF
        // got a reply from the wrong client
    }
}

// Writes out the given message to the network, waits for the reply, checks
// that the reply was an ACKNOWLEDGE, and doesn't return the reply.
void writeMsgAndExpectAck(Message msg) {
    Message reply = writeMsgMaster(msg);
    if(reply.message != ACKNOWLEDGED) {
        // TODO WOAH WTF
    }
}
