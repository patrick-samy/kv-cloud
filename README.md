## KV-Cloud ##
========

#### Build instructions ####
```sh
$ make [all | kvcloud | libkvcloud | client]
```

#### Testing ####
```sh
# initializing the filesystem with '-f' and starting server on default port 60000 (or use -p port)
$ ./kvcloud [-p port | -f ]
$ LD_LIBRARY_PATH=. ./client ip port
# sample program 'client' reads commands on the standard input and uses the API to make the associated requests to the server:
# get mykey myvalue
# set mykey test
# get mykey
# delete mykey
```

#### Authors ####
* Patrick Samy

