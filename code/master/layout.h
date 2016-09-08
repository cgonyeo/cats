#ifndef LAYOUT
#define LAYOUT

// A client in the system is either a Terminal or a Router. Well, or a vacuum,
// but that's handled differently.
typedef enum ClientType {
    Terminal,
    Router
} ClientType;

// A Client represents a client in this system. Clients are pinged by the
// master for updates, and clients are sent instructions by the master they
// must carry out (like changing tubes or alerting users).
struct Client {
    uint8_t     id;          // The id of this client
    ClientType  type;        // The type of this client
    const char *label;       // A human-readable label for this client
    int         numChildren; // The number of children this client has
    Client     *children;    // This client's children
};

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
const Client clientTree =
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

#endif
