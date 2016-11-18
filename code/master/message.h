#ifndef MESSAGE
#define MESSAGE

#include "shared/shared.h"

Message *writeMsgMaster(Message *msg);
void writeMsgAndExpectAck(Message *msg);

#endif
