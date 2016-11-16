#ifndef CONFIG
#define CONFIG

#include "path.h"
#include "request.h"
#include "client.h"
#include "message.h"

// The ID of the vacuum
#define VACUUM_ID 1

enum VacuumState {
    VPulling,
    VPushing
};

void markConfigComplete(uint8_t id);
void markPacketReached(uint8_t id);
void configureForPath(struct Path *p);
bool pathIsConfigured(struct Path *p);
void configVacuum(VacuumState state);
void startVacuum();
void stopVacuum();

#endif
