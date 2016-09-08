# CATS router

The router used in CATS is a device that can connect a tube to any of up to 4
other tubes. It is controlled by an arduino, that receives commands from a
master arduino over an RS485 bus.

When the system wishes to push to or pull from a given location, it can send a
command to each router along the path to connect the correct tube, providing a
continuous path from the vacuum to the target.
