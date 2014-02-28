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

void != echo >&2;

# disable MakeStats notice
BUILD_STATS_NOTICE=FALSE

# enable MakeStats auto commit; do not commit, just add the file to next commit
BUILD_STATS_AUTO_COMMIT ?= add

BUILD_FLAGS += \
-DBUILD_VERSION_VENDOR='"$(BUILD_VENDOR)"' \
-DBUILD_VERSION_TRIPLET='"$(BUILD_TRIPLET)"' \
-DBUILD_VERSION_DESCRIPTION='"$(BUILD_VENDOR) $(BUILD_NAME) $(BUILD_TRIPLET)"' \
-DBUILD_VERSION_NUMBER=$(BUILD_NUMBER)

BUILD_HOME ?= .
BUILD_SRC = $(BUILD_HOME)/src
BUILD_PKG = $(BUILD_HOME)/pkg
BUILD_TOOLS = $(BUILD_HOME)/mktools
BUILD_HYPER_VARIANT_PKG = $(BUILD_PKG)/HyperVariant

BUILD_OUTPUT ?= .
BUILD_BIN ?= bin

BUILD_PATHS = $(BUILD_BIN) $(BUILD_OUTPUT)

void != mkdir -p $(BUILD_PATHS); # make is sofaking stupid...

# System Install Paths for lib & header
SYSTEM_LIBDIR := /usr/local/lib
SYSTEM_INCDIR := /usr/local/include

CFLAGS = -g3

BUILD_ARCHIVE = $(BUILD_OUTPUT)/HashTable.a
BUILD_HEADER = $(BUILD_OUTPUT)/HashTable.h
BUILD_MAIN = $(BUILD_BIN)/HashTable.o
BUILD_SHARED = $(BUILD_OUTPUT)/libhashtable

BUILD_HYPER_VARIANT_MAIN = $(BUILD_BIN)/HyperVariant.o

# This MakeStats variable updates build revision if these files are modified
# We will also use this list as a prerequisite list for our main object.
BUILD_VERSION_SOURCES = $(BUILD_SRC)/HashTable.c $(BUILD_SRC)/HashTable.h \
	$(BUILD_HYPER_VARIANT_PKG)/src/HyperVariant.c

# define our targets (these are based on variables from the MakeStats include)
all: archive library demo

$(BUILD_PATHS):
	@mkdir -p $@

# include MakeStats
include $(BUILD_TOOLS)/MakeStats.mk

# create the shared object "meta-datum"
BUILD_SOFLAGS = -export-dynamic -shared -soname $(BUILD_SHARED).so.$(BUILD_MAJOR)
BUILD_LIBRARY:= $(BUILD_SHARED).so.$(BUILD_TRIPLET)

# Shortcuts: these allow us make build targets based on included variables
archive: $(BUILD_ARCHIVE)
library: $(BUILD_LIBRARY)
demo: $(BUILD_BIN)/demo

$(BUILD_HYPER_VARIANT_PKG)/src/HyperVariant.c: $(BUILD_HYPER_VARIANT_PKG)/src

$(BUILD_HYPER_VARIANT_PKG)/src: $(BUILD_HYPER_VARIANT_PKG)
	@tar -xzf $<-0.0.85.tar.gz -C $<

$(BUILD_HYPER_VARIANT_PKG):
	@mkdir $@

$(BUILD_HYPER_VARIANT_MAIN): $(BUILD_HYPER_VARIANT_PKG)
	@ \
	 BUILD_STATS_AUTO_COMMIT=FALSE \
	 BUILD_HOME=$(BUILD_HYPER_VARIANT_PKG) \
	 make --no-print-directory -f $(BUILD_HYPER_VARIANT_PKG)/Makefile \
	 bin/HyperVariant.o

$(BUILD_MAIN): CFLAGS += -fPIC $(BUILD_FLAGS) -I$(BUILD_HYPER_VARIANT_PKG)/src
$(BUILD_MAIN): $(BUILD_VERSION_SOURCES)
	@$(make-build-number)
	$(COMPILE.c) -o $@ $<
	@$(make-build-revision)
	@echo

$(BUILD_HEADER): $(BUILD_SRC)/HashTable.h $(BUILD_OUTPUT)
	@cp $< $@

$(BUILD_ARCHIVE): $(BUILD_MAIN) $(BUILD_HEADER) $(BUILD_HYPER_VARIANT_MAIN)
	@$(make-build-number)
	@echo -e 'Building $(BUILD_NAME) $(BUILD_TRIPLET) archive...\n'
	$(AR) -vr $@ $< $(BUILD_HYPER_VARIANT_MAIN)
	@echo

$(BUILD_LIBRARY): $(BUILD_MAIN) $(BUILD_HYPER_VARIANT_MAIN)
	@$(make-build-number)
	@echo -e 'Building $(BUILD_NAME) $(BUILD_TRIPLET) library...\n'
	ld $(BUILD_SOFLAGS) -o $@ $< $(BUILD_HYPER_VARIANT_MAIN)
	@echo

$(BUILD_BIN)/demo.o: $(BUILD_SRC)/demo.c
	@echo -e 'Building $(BUILD_NAME) $(BUILD_TRIPLET) demo...\n'
	$(COMPILE.c) -o $@ $<
	@echo

$(BUILD_BIN)/demo: $(BUILD_BIN)/demo.o $(BUILD_MAIN) $(BUILD_HYPER_VARIANT_MAIN)
	$(LINK.c) -o $@ $^
	@echo

install: $(BUILD_SHARED) $(BUILD_HEADER)
	@echo 'Installing shared library...'
	@cp -v $(BUILD_SHARED) $(SYSTEM_LIBDIR)
	@cp -v $(BUILD_HEADER) $(SYSTEM_INCDIR)
	ldconfig -n $(SYSTEM_LIBDIR)
	@echo

clean:
	@$(RM) -rv $(BUILD_MAIN) $(BUILD_ARCHIVE) $(BUILD_HEADER) $(BUILD_SHARED)* \
		$(BUILD_BIN)/demo $(BUILD_BIN)/demo.o $(BUILD_HYPER_VARIANT_MAIN)
	@echo

.DEFAULT_GOAL = all
.SUFFIXES:
.PHONY: all archive library demo
