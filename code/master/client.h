#ifndef CLIENT
#define CLIENT

#include "shared/shared.h"
#include "layout.h"

// flattenedClientTree and numClients are both generated during program setup
// from the clientTree. The names are hopefully self-explanatory.
extern Client *flattenedClientTree;
extern int numClients;

void walkTree(const Client *tree, void (*walkFunc) (const Client *, void *), void *data);
void clientCounter(const Client *c, void *data);
void flattener(const Client *c, void *data);
void initClients();
bool hasChild(uint8_t id, const Client *tree);

#endif
