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

# disable MakeStats notice
BUILD_STATS_NOTICE=FALSE

BUILD_FLAGS += \
-DHT_VERSION_VENDOR='"$(BUILD_VENDOR)"' \
-DHT_VERSION_TRIPLET='"$(BUILD_TRIPLET)"' \
-DHT_VERSION_DESCRIPTION='"$(BUILD_VENDOR) $(BUILD_NAME) $(BUILD_TRIPLET)"' \
-DHT_VERSION_BUILDNO=$(BUILD_NUMBER)

BUILD_OUTPUT ?= .
BUILD_SRC ?= src
BUILD_BIN ?= bin

void != echo >&2;

void != test -d $(BUILD_BIN) || mkdir $(BUILD_BIN);
void != test -d $(BUILD_OUTPUT) || mkdir $(BUILD_OUTPUT);

ARCHIVE = $(BUILD_OUTPUT)/HashTable.a
SHARED = $(BUILD_OUTPUT)/libhashtable.so

HEADER = $(BUILD_OUTPUT)/HashTable.h
BUILD_MAIN = $(BUILD_BIN)/HashTable.o

CFLAGS = -O3
BUILD_SOFLAGS = -export-dynamic -shared -soname $(SHARED).$(BUILD_MAJOR)

all: $(ARCHIVE) $(SHARED)

# This MakeStats variable updates build revision if these files are modified
# We will also use this list as a prerequisite list for our main object.
BUILD_VERSION_SOURCES = $(BUILD_SRC)/HashTable.c $(BUILD_SRC)/HashTable.h

include mktools/MakeStats.mk

$(BUILD_MAIN): CFLAGS += -fPIC

# Notice push-build below, it triggers ALL MakeStats updates
$(BUILD_MAIN): $(BUILD_VERSION_SOURCES) push-build
	$(COMPILE.c) $(BUILD_MAIN_FLAGS) $(BUILD_FLAGS) -o $@ $<
	@echo

$(HEADER): $(BUILD_OUTPUT)
	@cp $@ $^

$(ARCHIVE): $(BUILD_MAIN) $(HEADER)
	@echo -e Building $(BUILD_NAME) $(BUILD_TRIPLET) archive...'\n' >&2;
	$(AR) -vr $@ $<
	@echo

$(SHARED).$(BUILD_TRIPLET): $(BUILD_MAIN)
	@echo -e Building $(BUILD_NAME) $(BUILD_TRIPLET) library...'\n' >&2;
	ld $(BUILD_SOFLAGS) -o $@ $<
	@echo

shared: $(SHARED).$(BUILD_TRIPLET)
demo: $(BUILD_BIN)/demo

$(BUILD_BIN)/demo.o: $(BUILD_SRC)/demo.c
	@echo -e Building $(BUILD_NAME) $(BUILD_TRIPLET) demo...'\n' >&2;
	$(COMPILE.c) -o $@ $<
	@echo

$(BUILD_BIN)/demo: $(BUILD_BIN)/demo.o  $(BUILD_MAIN)
	$(LINK.c) -o $@ $^
	@echo

clean:
	@$(RM) -v $(BUILD_MAIN) $(ARCHIVE) $(HEADER) $(SHARED)* \
		$(BUILD_BIN)/demo $(BUILD_BIN)/demo.o
	@echo
