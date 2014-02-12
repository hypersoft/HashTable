CFLAGS = -O3

OUTPUT ?= .

SRCDIR ?= src

void != test -d bin || mkdir bin;

ARCHIVE = $(OUTPUT)/HashTable.a

all: $(ARCHIVE)

bin/HashTable.o: $(SRCDIR)/HashTable.c $(SRCDIR)/HashTable.h
	$(COMPILE.c) -o $@ $<

$(ARCHIVE): bin/HashTable.o
	$(AR) -vr $@ $^
	@$(push-stats)

bin/demo.o: $(SRCDIR)/demo.c
	$(COMPILE.c) -o $@ $^

bin/demo: bin/demo.o  bin/HashTable.o
	$(LINK.c) -o $@ $^

clean:
	$(RM) bin/HashTable.o $(ARCHIVE)

NOTICE=FALSE
include mktools/MakeStats.mk
