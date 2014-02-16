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

# enable MakeStats auto commit; Warning noisy commit history!
BUILD_STATS_AUTO_COMMIT = TRUE

BUILD_FLAGS += \
-DHT_VERSION_VENDOR='"$(BUILD_VENDOR)"' \
-DHT_VERSION_TRIPLET='"$(BUILD_TRIPLET)"' \
-DHT_VERSION_DESCRIPTION='"$(BUILD_VENDOR) $(BUILD_NAME) $(BUILD_TRIPLET)"' \
-DHT_VERSION_BUILDNO=$(BUILD_NUMBER)

BUILD_OUTPUT ?= .
BUILD_SRC ?= src
BUILD_BIN ?= bin

# System Install Paths for lib & header
SYSTEM_LIBDIR := /usr/local/lib
SYSTEM_INCDIR := /usr/local/include

void != echo >&2;

void != test -d $(BUILD_BIN) || mkdir $(BUILD_BIN);
void != test -d $(BUILD_OUTPUT) || mkdir $(BUILD_OUTPUT);

CFLAGS = -O3

BUILD_ARCHIVE = $(BUILD_OUTPUT)/HashTable.a
BUILD_HEADER = $(BUILD_OUTPUT)/HashTable.h
BUILD_MAIN = $(BUILD_BIN)/HashTable.o
BUILD_SHARED = $(BUILD_OUTPUT)/libhashtable

# This MakeStats variable updates build revision if these files are modified
# We will also use this list as a prerequisite list for our main object.
BUILD_VERSION_SOURCES = $(BUILD_SRC)/HashTable.c $(BUILD_SRC)/HashTable.h

# build these targets even if files exist
.PHONY: all archive library demo

# This project doesn't use old implicit rules
.SUFFIXES:

# define our targets (these are based on variables from the MakeStats include)
all: archive library demo

# include MakeStats
include $(BUILD_SRC)/../mktools/MakeStats.mk

# create the shared object "meta-datum"
BUILD_SOFLAGS = -export-dynamic -shared -soname $(BUILD_SHARED).so.$(BUILD_MAJOR)
BUILD_LIBRARY:= $(BUILD_SHARED).so.$(BUILD_TRIPLET)

# Shortcuts: these allow us make build targets based on included variables
archive: $(BUILD_ARCHIVE)
library: $(BUILD_LIBRARY)
demo: $(BUILD_BIN)/demo

$(BUILD_MAIN): CFLAGS += -fPIC $(BUILD_FLAGS)
$(BUILD_MAIN): $(BUILD_VERSION_SOURCES)
	@$(make-build-number)
	$(COMPILE.c) -o $@ $<
	@$(make-build-revision)
	@echo

$(BUILD_HEADER): $(BUILD_SRC)/HashTable.h
	@cp $< $@

$(BUILD_ARCHIVE): $(BUILD_MAIN) $(BUILD_HEADER)
	@$(make-build-number)
	@echo -e 'Building $(BUILD_NAME) $(BUILD_TRIPLET) archive...\n'
	$(AR) -vr $@ $<
	@echo

$(BUILD_LIBRARY): $(BUILD_MAIN)
	@$(make-build-number)
	@echo -e 'Building $(BUILD_NAME) $(BUILD_TRIPLET) library...\n'
	ld $(BUILD_SOFLAGS) -o $@ $<
	@echo

$(BUILD_BIN)/demo.o: $(BUILD_SRC)/demo.c
	@echo -e 'Building $(BUILD_NAME) $(BUILD_TRIPLET) demo...\n'
	$(COMPILE.c) -o $@ $<
	@echo

$(BUILD_BIN)/demo: $(BUILD_BIN)/demo.o  $(BUILD_MAIN)
	$(LINK.c) -o $@ $^
	@echo

install: $(BUILD_SHARED) $(BUILD_HEADER)
	@echo 'Installing shared library...'
	@cp -v $(BUILD_SHARED) $(SYSTEM_LIBDIR)
	@cp -v $(BUILD_HEADER) $(SYSTEM_INCDIR)
	ldconfig -n $(SYSTEM_LIBDIR)
	@echo

clean:
	@$(RM) -v $(BUILD_MAIN) $(BUILD_ARCHIVE) $(BUILD_HEADER) $(BUILD_SHARED)* \
		$(BUILD_BIN)/demo $(BUILD_BIN)/demo.o
	@echo
