CFLAGS = -O3

BUILD_OUTPUT ?= .

BUILD_SRC ?= src

BUILD_BIN ?= bin

void != test -d $(BUILD_BIN) || mkdir $(BUILD_BIN);

ARCHIVE = $(BUILD_OUTPUT)/HashTable.a
HEADER = $(BUILD_OUTPUT)/HashTable.h

all: $(ARCHIVE)

$(BUILD_BIN)/HashTable.o: $(BUILD_SRC)/HashTable.c $(BUILD_SRC)/HashTable.h
	$(COMPILE.c) -o $@ $<

$(HEADER):
	@cp $(BUILD_SRC)/HashTable.h $(BUILD_OUTPUT)

$(ARCHIVE): $(BUILD_BIN)/HashTable.o $(HEADER)
	@echo -e '\n'Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(THIS_BUILD_REVISION) archive...'\n' >&2;
	$(AR) -vr $@ $<
	@$(push-stats)

$(BUILD_BIN)/demo.o: $(BUILD_SRC)/demo.c
	@echo -e '\n'Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION) demo...'\n' >&2;
	$(COMPILE.c) -o $@ $^

$(BUILD_BIN)/demo: $(BUILD_BIN)/demo.o  $(BUILD_BIN)/HashTable.o
	$(LINK.c) -o $@ $^

clean:
	@$(RM) -v $(BUILD_BIN)/HashTable.o $(ARCHIVE) $(HEADER) \
		$(BUILD_BIN)/demo $(BUILD_BIN)/demo.o

NOTICE=FALSE
include mktools/MakeStats.mk
