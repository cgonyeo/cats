#ifndef CATS_SHARED
#define CATS_SHARED

#include <Arduino.h>
#include <SoftwareSerial.h>

#define MASTER_ID 0

// Stuff for the RS485 communications
#define SSerialRX        10  //Serial Receive pin
#define SSerialTX        11  //Serial Transmit pin
#define SSerialTxControl 3   //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW
#define Pin13LED         13
extern SoftwareSerial RS485Serial; // RX, TX

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

// A Message represents a message sent over the wire.
struct Message {
    uint8_t id;      // The sender of the message
    uint8_t message; // The message type (see #define's earlier in this file)
    uint8_t data;    // An arbitrary byte of data, meaning dependent on type
};

void writeMsg(Message msg);
Message getMsg();

#endif
