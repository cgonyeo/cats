# Arduinos

CATS could be described as a 1950s network, but to be fully automated (as in,
not require human interaction during typical operation) it has a 1970s era
network overlayed on top of it.

Everywhere the tubes go, there is also a serial bus (RS485), that is used to
receive user requests and send instructions to the different components.

The arduinos operate in a master/client model, with one master arduino that
tracks the state of the system and many clients that only hold state related to
themselves.

To avoid network collisions, the clients only speak after being sent a message
by the master. Since sometimes clients generate events independent of the
master (user requests, detected a capsule go by) this means that the master
will frequently individually ping each client asking for an update.

Each message is 3 bytes. Messages from the master are comprised of the master's
ID, the message, and the target's ID. Messages from a client are comprised of
the client's ID, the message, and a byte of data whose meaning varies based on
the message.
