# Makefile

# Specify the compiler
CC := gcc

# Flags for compilation
CFLAGS := -Wall -Wextra

# Targets and their dependencies
all: brute hash

brute: brute.c
	$(CC) $(CFLAGS) $< -o $@

hash: hash.c
	$(CC) $(CFLAGS) $< -o $@

# Specify the default target
.DEFAULT_GOAL := all

# Clean rule to remove generated binaries
clean:
	rm -f brute hash

