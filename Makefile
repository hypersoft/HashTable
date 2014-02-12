CFLAGS = -O3

OUTPUT ?= .

SRCDIR ?= src

BIN ?= bin

void != test -d $(BIN) || mkdir $(BIN);

ARCHIVE = $(OUTPUT)/HashTable.a
HEADER = $(OUTPUT)/HashTable.h

all: $(ARCHIVE)

$(BIN)/HashTable.o: $(SRCDIR)/HashTable.c $(SRCDIR)/HashTable.h
	$(COMPILE.c) -o $@ $<

$(HEADER):
	@cp $(SRCDIR)/HashTable.h $(OUTPUT)

$(ARCHIVE): $(BIN)/HashTable.o $(HEADER)
	@echo -e '\n'Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(THIS_BUILD_REVISION) archive...'\n' >&2;
	$(AR) -vr $@ $<
	@$(push-stats)

$(BIN)/demo.o: $(SRCDIR)/demo.c
	@echo -e '\n'Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION) demo...'\n' >&2;
	$(COMPILE.c) -o $@ $^

$(BIN)/demo: $(BIN)/demo.o  $(BIN)/HashTable.o
	$(LINK.c) -o $@ $^

clean:
	@$(RM) -v $(BIN)/HashTable.o $(ARCHIVE) $(HEADER) $(BIN)/demo $(BIN)/demo.o

NOTICE=FALSE
include mktools/MakeStats.mk
