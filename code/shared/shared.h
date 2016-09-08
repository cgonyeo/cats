#ifndef CATS_SHARED
#define CATS_SHARED

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

#endif
