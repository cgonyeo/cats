#include <SoftwareSerial.h>

#include "shared/shared.h"
#include "client.h"
#include "path.h"
#include "request.h"
#include "config.h"

// The ID of this arduino
#define MY_ID 0

// Stuff for the RS485 communications
#define SSerialRX        10  //Serial Receive pin
#define SSerialTX        11  //Serial Transmit pin
#define SSerialTxControl 3   //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW
#define Pin13LED         13
SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

// A Message represents a message sent over the wire.
struct Message {
    uint8_t id;      // The sender of the message
    uint8_t message; // The message type (see #define's earlier in this file)
    uint8_t data;    // An arbitrary byte of data, meaning dependent on type
};


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

    initClients();

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
                Request *requestToCancel = getRequest(resp.id);
                if(requestToCancel == NULL) {
                    // This request doesn't exist! Let's just ignore this...
                    break;
                }
                if(requestToCancel->state == Queued ||
                   requestToCancel->state == ConfigForPull ||
                   requestToCancel->state == ReadyToPull) {
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
                            .data = requestToCancel->to
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
        Request *curr = currentRequest();
        switch(curr->state) {
            case Queued:
                // The top request is queued, let's send out configuration
                // instructions.
                configureForPath(curr->fromPath);
                configVacuum(VPulling);
                curr->state = ConfigForPull;
                break;
            case ConfigForPull: {
                // The top request is being configured for pulling. Let's check
                // if it's done, and start pulling if it is.
                if(pathIsConfigured(curr->fromPath)) {
                    // TODO check if vacuum is configured
                    startVacuum();
                    writeMsgAndExpectAck(
                        {
                            .id = MY_ID,
                            .message = READY_TO_PULL,
                            .data = curr->from
                        }
                    );
                    curr->state = ReadyToPull;
                }
                break;
            }
            case ReadyToPull:
                // The top request is pulling, but the capsule hasn't entered
                // the system yet. Let's check for the capsule, and make this
                // request non-cancellable if we've seen it.
                if(curr->packetStarted) {
                    curr->state = Pulling;
                }
                break;
            case Pulling:
                // The top request is pulling. Check if the capsule has reached
                // the root of the system, and if it has stop the vacuum and
                // configure the system for pushing.
                if(curr->fromPath.pathNodes[0].packetReached) {
                    stopVacuum();
                    configureForPath(curr->toPath);
                    configVacuum(VPushing);
                    curr->state = ConfigForPush;
                }
                break;
            case ConfigForPush:
                // The top request is being configured for pushing. Let's check
                // if it's done, and start pushing if it is.
                if(pathIsConfigured(curr->toPath)) {
                    // TODO check if vacuum is configured
                    startVacuum();
                    curr->state = Pushing;
                }
                break;
            case Pushing:
                // The system is pushing. Let's check if the capsule has
                // reached its destination, and stop pushing if it has.
                if(curr->packetFinished) {
                    stopVacuum();
                    curr->state = Finished;
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
