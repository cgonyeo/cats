#include "shared/shared.h"

// The ID of this arduino
#define MY_ID 3

// setup, as per arduino tooling, runs once when the arduino starts.
void setup()
{
    Serial.begin(9600);

    Serial.print("I'm a terminal with id ");
    Serial.println(MY_ID);

    pinMode(SSerialTxControl, OUTPUT);    
    digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver   

    // Start the software serial port, to another device
    RS485Serial.begin(4800);   // set the data rate 
}

// loop, as per arduino tooling, runs repeatedly after setup is complete.
void loop()
{
    Message msg = getMsg();

    if(msg.id != MASTER_ID) {
        // ignore messages not from the master
        return;
    }

    if(msg.data != MY_ID) {
        // ignore messages not addressed to me
    }

    switch(msg.message) {
        case STATUS_UPDATE:
            break;
        case INCOMING_CAPSULE:
            break;
        case CLEAR_INCOMING:
            break;
        case SYSTEM_OFFLINE:
            break;
        case CLEAR_OFFLINE:
            break;
        case SYSTEM_ERROR:
            break;
        case CLEAR_ERROR:
            break;
        case READY_TO_PULL:
            break;
        case CLEAR_READY:
            break;
        case REQUEST_QUEUED:
            break;
        case CLEAR_QUEUED:
            break;
        default:
            // TODO WOAH WTF
            // got a message I don't know how to handle!
            break;
    }
}
