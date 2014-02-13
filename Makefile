CFLAGS = -O3

BUILD_VENDOR = Hypersoft Systems

BUILD_FLAGS += \
-DHT_VERSION_VENDOR='"$(BUILD_VENDOR)"' \
-DHT_VERSION_TRIPLET='"$(THIS_BUILD_TRIPLET)"' \
-DHT_VERSION_DESCRIPTION='"$(BUILD_VENDOR) $(BUILD_NAME) $(THIS_BUILD_TRIPLET)"' \
-DHT_VERSION_BUILDNO=$(THIS_BUILD_NUMBER)

BUILD_OUTPUT ?= .
BUILD_SRC ?= src
BUILD_BIN ?= bin

void != echo >&2;

void != test -d $(BUILD_BIN) || mkdir $(BUILD_BIN);
void != test -d $(BUILD_OUTPUT) || mkdir $(BUILD_OUTPUT);

ARCHIVE = $(BUILD_OUTPUT)/HashTable.a
HEADER = $(BUILD_OUTPUT)/HashTable.h

all: $(ARCHIVE)

$(BUILD_BIN)/HashTable.o: $(BUILD_SRC)/HashTable.c $(BUILD_SRC)/HashTable.h
	$(COMPILE.c) $(BUILD_FLAGS) -o $@ $<
	@echo

$(HEADER):
	@cp $(BUILD_SRC)/HashTable.h $(BUILD_OUTPUT)

$(ARCHIVE): $(BUILD_BIN)/HashTable.o $(HEADER)
	@echo -e Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(THIS_BUILD_REVISION) archive...'\n' >&2;
	$(AR) -vr $@ $<
	@$(push-stats)
	@echo

$(BUILD_BIN)/demo.o: $(BUILD_SRC)/demo.c
	@echo -e Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION) demo...'\n' >&2;
	$(COMPILE.c) -o $@ $^
	@echo

$(BUILD_BIN)/demo: $(BUILD_BIN)/demo.o  $(BUILD_BIN)/HashTable.o
	$(LINK.c) -o $@ $^
	@echo

clean:
	@$(RM) -v $(BUILD_BIN)/HashTable.o $(ARCHIVE) $(HEADER) \
		$(BUILD_BIN)/demo $(BUILD_BIN)/demo.o
	@echo

NOTICE=FALSE
include mktools/MakeStats.mk
