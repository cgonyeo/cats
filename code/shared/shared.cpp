#include "shared.h"

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

void writeMsg(Message msg) {
    digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit   
    RS485Serial.write(msg.id);
    RS485Serial.write(msg.message);
    RS485Serial.write(msg.data);
    digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit       
}

Message getMsg() {
    int bytesReceived = 0;
    Message receivedMsg;
    while(true) {
        if(RS485Serial.available()) {
            uint8_t data = RS485Serial.read();
            switch(bytesReceived) {
                case 0:
                    receivedMsg.id = data;
                    break;
                case 1:
                    receivedMsg.message = data;
                    break;
                case 2:
                    receivedMsg.data = data;
                    return receivedMsg;
            };
            bytesReceived++;
        }
    }
}
