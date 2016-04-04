
CC = gcc
OBJECTS = rules.o hcre.o
BINARIES = hcre
DEBUGS =

COMPILE = $(CC) -O2 -std=c99 -march=native $(CFLAGS) $(DEBUGS) -Wall -Wextra -funsigned-char -Wno-pointer-sign -Wno-sign-compare


all: $(BINARIES)

%.o: %.c
	$(COMPILE) -c $< -o $@

hcre: rules.o hcre.o
	$(COMPILE) $^ -o hcre

debug: DEBUGS = -DDEBUG_PARSING -DDEBUG_DUPES -DDEBUG_STATS -DDEBUG_OUTPUT
debug: hcre

clean:
	$(RM) $(BINARIES) $(OBJECTS)
