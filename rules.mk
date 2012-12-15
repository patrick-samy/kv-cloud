BINARY      := kvcloud

# Sub-directories
CPPFLAGS    += -Iinclude -Isrc
SOURCES     := src/main.c src/server.c

OBJS        := $(SOURCES:%.c=%.o)
DEPS        := $(SOURCES:%.c=%.d)
CLEAN       += $(DEPS) $(OBJS) $(BINARY)

# Build binary
$(BINARY): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

BINARIES    := $(BINARY)
