# Build tools
CC 	    := gcc
LD 	    := gcc

# Deactivate default rules and show warnings
MAKEFLAGS   := --no-builtin-rules --no-builtin-variables --warn-undefined-variables

# Contains the files to clean
CLEAN       :=

# Flags
CPPFLAGS    ?=
CFLAGS      ?=
LDFLAGS     ?=

#Default rule
all: targets

# Include the top-level rules.mk
include rules.mk

# Generic rules
%.o: %.c
	$(CC) -MD $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Misc rules
.SUFFIXES:

.PHONY: targets
targets: $(BINARIES)

.PHONY: clean
clean:
	rm -rf $(CLEAN)

