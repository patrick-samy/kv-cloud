############
## Server ##
############
BINARY                  := kvcloud
$(BINARY): CPPFLAGS     += -Iinclude -Isrc
# Add sources
SOURCES                 := src/main.c src/server.c src/storage.c

OBJS                    := $(SOURCES:%.c=%.o)
DEPS                    := $(SOURCES:%.c=%.d)
CLEAN                   += $(DEPS) $(OBJS) $(BINARY)

# Build binary
$(BINARY): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

#############
## Library ##
#############
API_SHARED              := libkvcloud.so
$(API_SHARED): CPPFLAGS += -Iinclude
$(API_SHARED): CFLAGS   += -fPIC

# Add sources
SOURCES                 := lib/api.c

OBJS                    := $(SOURCES:%.c=%.o)
DEPS                    := $(SOURCES:%.c=%.d)
CLEAN                   += $(DEPS) $(OBJS) $(API_SHARED)

# Build shared library
$(API_SHARED): $(OBJS)
	$(LD) -shared -o $@ $^

# Helper target
lib: $(API_SHARED)

##########
# Sample #
##########
SAMPLE                  := client
$(SAMPLE): CPPFLAGS     += -Iinclude -Isample

# Add sources
SOURCES                 := sample/client.c

OBJS                    := $(SOURCES:%.c=%.o) $(API_SHARED)
DEPS                    := $(SOURCES:%.c=%.d)
CLEAN                   += $(DEPS) $(OBJS) $(SAMPLE)

# Build binary
$(SAMPLE): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

#############
## Default ##
#############
BINARIES                := $(BINARY) $(API_SHARED) $(SAMPLE)

