#include "shared/shared.h"

// The ID of this arduino
#define MY_ID 3

struct StatusUpdate {
    uint8_t message;
    uint8_t data;
};

int numStatusUpdates = 0;
StatusUpdate *statusUpdates = NULL;

void queueStatusUpdate(StatusUpdate u) {
    numStatusUpdates++;
    statusUpdates = (StatusUpdate *)realloc(statusUpdates, numStatusUpdates);
    memcpy(&statusUpdates[numStatusUpdates-1], &u, sizeof(StatusUpdate));
}

StatusUpdate getStatusUpdate() {
    StatusUpdate u;
    memcpy(statusUpdates, &u, sizeof(StatusUpdate));

    numStatusUpdates--;
    memcpy(statusUpdates, statusUpdates++, numStatusUpdates * sizeof(StatusUpdate));
    statusUpdates = (StatusUpdate *)realloc(statusUpdates, numStatusUpdates);

    return u;
}

// setup, as per arduino tooling, runs once when the arduino starts.
void setup()
{
    Serial.begin(9600);

    Serial.print("I'm a router with id ");
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
            if(numStatusUpdates == 0) {
                writeMsg(
                    {
                        .id = MY_ID,
                        .message = NOTHING,
                        .data = 0
                    }
                );
            } else {
                StatusUpdate s = getStatusUpdate();
                writeMsg(
                    {
                        .id = MY_ID,
                        .message = s.message,
                        .data = s.data
                    }
                );
            }
            break;
        case CHANGE_TO_TUBE_0:
            break;
        case CHANGE_TO_TUBE_1:
            break;
        case CHANGE_TO_TUBE_2:
            break;
        case CHANGE_TO_TUBE_3:
            break;
        default:
            // TODO WOAH WTF
            // got a message I don't know how to handle!
            break;
    }
}
