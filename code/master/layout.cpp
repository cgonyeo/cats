#include <avr/pgmspace.h>
#include "layout.h"

// The clientTree is the tree of all clients available in the system. It is
// used by the master as the list of clients to ping for updates, and the
// structure of the tree is used to determine routing paths.
//
// Not included in the tree is the vacuum node, which is treated somewhat
// differently during routing decisions.
//
// On program startup the flattenedClientTree variable is created to point to a
// flattened version of this tree, which is easier to iterate over.
//
// All leaf nodes must be of type Terminal, and all non-leaf nodes must be of
// type router.
//
// Care must be taken to ensure that for each node, numChildren is accurate. If
// it's too low parts of the tree will be ignored during routing decisions, and
// if it's too high undefined behavior will creep in.
//
// The label field is simply used to improve the human readability of log
// messages.
//
// Each id _must_ be unique! Ids 0 and 1 are reserved for the master and the
// vacuum.
const PROGMEM Client clientTree =
    {
        .id = 2,
        .type = Router,
        .label = "root router",
        .numChildren = 2,
        .children = (Client[]) {
            {
                .id = 3,
                .type = Terminal,
                .label = "terminal 1",
                .numChildren = 0,
                .children = NULL
            },
            {
                .id = 4,
                .type = Terminal,
                .label = "router 2",
                .numChildren = 1,
                .children = (Client[]) {
                    {
                        .id = 5,
                        .type = Terminal,
                        .label = "terminal 2",
                        .numChildren = 0,
                        .children = NULL
                    }
                }
            }
        }
    };
