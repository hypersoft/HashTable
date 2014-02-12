CFLAGS = -O3

OUTPUT ?= .

SRCDIR ?= src

void != test -d bin || mkdir bin;

ARCHIVE = $(OUTPUT)/HashTable.a
HEADER = $(OUTPUT)/HashTable.h

all: $(ARCHIVE)

bin/HashTable.o: $(SRCDIR)/HashTable.c $(SRCDIR)/HashTable.h
	$(COMPILE.c) -o $@ $<

$(HEADER):
	@cp $(SRCDIR)/HashTable.h $(OUTPUT)

$(ARCHIVE): bin/HashTable.o $(HEADER)
	$(AR) -vr $@ $<
	@$(push-stats)

bin/demo.o: $(SRCDIR)/demo.c
	$(COMPILE.c) -o $@ $^

bin/demo: bin/demo.o  bin/HashTable.o
	$(LINK.c) -o $@ $^

clean:
	$(RM) bin/HashTable.o $(ARCHIVE) $(HEADER)

NOTICE=FALSE
include mktools/MakeStats.mk
