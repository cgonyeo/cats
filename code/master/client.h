#ifndef CLIENT
#define CLIENT

#include "shared/shared.h"
#include "layout.h"

// flattenedClientTree and numClients are both generated during program setup
// from the clientTree. The names are hopefully self-explanatory.
extern Client *flattenedClientTree;
extern int numClients;

void walkTree(Client tree, void (*walkFunc) (Client, void *), void *data);
void clientCounter(Client c, void *data);
void flattener(Client c, void *data);
void initClients();
bool hasChild(uint8_t id, Client tree);

#endif
