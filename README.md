# rtifsrd
## Real-time Interface Statistics Receiver Daemon
rtifsrd is subscriber|syncer|proxy daemon that receives data from the sender (rtifssd).\
It is a small part of my ISP management project.

### Supported platforms
FreeBSD and Linux

### Usage
Inteended to be used with daemon(8) and RC scripts\
Arguments:\
-s for subscriber socket e.g. tcp://10.0.0.254:5555\
-d for publisher socket e.g. same pattern as above\
-i for message forwarding interval in seconds (normally set to 1)

### Dependencies
ZeroMQ version 4 library

### TODO
Extensive testing\
Further development\
Make it more configurable\
Improve code quality
