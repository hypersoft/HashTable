BUILD_VENDOR = Hypersoft Systems

# WHEN TO: make push-major;
#
# The behavior of a function changes so that it no longer meets its original
# specification.
#
# Exported data items change (exception: adding optional items to the ends of 
# structures is not warranting, as long as those structures are only allocated
# within the library).
#
# An exported function is removed.
#
# The interface of an exported function changes.
#
# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html

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
SHARED = $(BUILD_OUTPUT)/libhashtable.so
HEADER = $(BUILD_OUTPUT)/HashTable.h

CFLAGS = -O3
BUILD_OFLAGS = -fPIC
BUILD_SOFLAGS = -export-dynamic -shared -soname $(SHARED).$(BUILD_MAJOR)

all: $(ARCHIVE)

NOTICE=FALSE
include mktools/MakeStats.mk

make.sts: $(BUILD_SRC)/HashTable.c $(BUILD_SRC)/HashTable.h
ifndef BUILD_TEST
	@$(push-stats)
endif

$(BUILD_BIN)/HashTable.o: \
		$(BUILD_SRC)/HashTable.c $(BUILD_SRC)/HashTable.h make.sts
	$(COMPILE.c) $(BUILD_OFLAGS) $(BUILD_FLAGS) -o $@ $<
	@echo

$(HEADER):
	@cp $(BUILD_SRC)/HashTable.h $(BUILD_OUTPUT)

$(ARCHIVE): $(BUILD_BIN)/HashTable.o $(HEADER)
	@echo -e Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(THIS_BUILD_REVISION) archive...'\n' >&2;
	$(AR) -vr $@ $<
	@echo

$(SHARED).$(THIS_BUILD_TRIPLET): $(BUILD_BIN)/HashTable.o make.sts
	@echo -e Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(THIS_BUILD_REVISION) library...'\n' >&2;
	ld $(BUILD_SOFLAGS) -o $@ $<
	@echo

shared: $(SHARED).$(THIS_BUILD_TRIPLET)

$(BUILD_BIN)/demo.o: $(BUILD_SRC)/demo.c
	@echo -e Building $(BUILD_NAME) \
		$(BUILD_MAJOR).$(BUILD_MINOR).$(THIS_BUILD_REVISION) demo...'\n' >&2;
	$(COMPILE.c) -o $@ $^
	@echo

$(BUILD_BIN)/demo: $(BUILD_BIN)/demo.o  $(BUILD_BIN)/HashTable.o
	$(LINK.c) -o $@ $^
	@echo

clean:
	@$(RM) -v $(BUILD_BIN)/HashTable.o $(ARCHIVE) $(HEADER) $(SHARED)* \
		$(BUILD_BIN)/demo $(BUILD_BIN)/demo.o
	@echo
