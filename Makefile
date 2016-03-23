
CC = gcc
OBJECTS = rules.o hcre.o
BINARIES = hcre

COMPILE = $(CC) -O2 -std=c99 -march=native $(CFLAGS) -Wall -Wextra -funsigned-char -Wno-pointer-sign -Wno-sign-compare -Wno-char-subscripts


all: $(BINARIES)

%.o: %.c
	$(COMPILE) -c $< -o $@

hcre: rules.o hcre.o
	$(COMPILE) $^ -o hcre

clean:
	$(RM) $(BINARIES) $(OBJECTS)
