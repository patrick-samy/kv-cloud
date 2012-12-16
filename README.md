## KV-Cloud ##
========

#### Build instructions ####
```sh
$ make [all | kvcloud | libkvcloud | client]
```

#### Testing ####
```sh
# initializing the filesystem and starting server on default port 60000 (or use -p port)
$ ./kvcloud -f
$ LD_LIBRARY_PATH=. ./client 0.0.0.0 60000
```

#### Authors ####
* Patrick Samy

