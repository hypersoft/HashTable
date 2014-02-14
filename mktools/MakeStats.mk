# ==================================================================================
#
# This software is provided TO YOU without any warranty, guarantee, or liability
# resulting in use, misuse, abuse or distribution.
#
# YOU ARE FREE TO COPY, DISTRIBUTE OR MODIFY THIS CODE FREE OF CHARGE, PENALTY,
# OR DONATION PROVIDED THE FOLLOWING CONDITIONS ARE MET:
#
#  1) DO NOT REMOVE OR MODIFY THE PRECEDING NOTICE.
#  2) DO NOT REMOVE OR MODIFY THE FOLLOWING NOTICE.
#

ifneq (FALSE, $(BUILD_STATS_NOTICE))
BUILD_STATS_NOTICE := $(shell { \
	printf 'Makestats: \n\n'; \
	printf '\t%s\n' \
		'(C) 2014 Hypersoft Systems All Rights Reserved.' \
		'(C) 2014 Triston J. Taylor <pc.wiz.tt@gmail.com>' \
	; \
	echo; \
} >&2)
endif

#
# ==================================================================================
#
# You may modify the license in any way you wish provided the following conditions
# are met:
#
#  1) You obtain written permission from <pc.wiz.tt@gmail.com>.
#  2) You make a contribution of $5.00 or more to support the ORIGINAL works of THIS
#     copyright holder.
#
# PayPal: via e-mail or via web
# https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=DG3H6F8DSG4BC
#
# ==================================================================================
#    Please indicate "MakeStats" as your purpose and provide ANY contact address.
# ==================================================================================

# Usage
#
# To use this file properly, you must include the file at the end
# of your make script, or at least after the default rule.
#
# Variable: BUILD_VERSION_SOURCES
#
# Populate this variable with all sources that will cause version to change
# when modified.
#
# To trigger update of stats, use push-build as a final prerequisite:
#
# my_program.o: some.c some.h push-build
#
# Which will automatically increment your build number, and if ANY
# BUILD_VERSION_SOURCES are new, will also update build revision, user and date.
#
# To increment the minor build version, from the command line you shall issue:
#
# make push-minor;
#
# Which will handle the versioning for you.
#
# Likewise to increment the major build version, from the command line:
#
# make push-major;
#
# To change the product, project, or code name:
#
# make build-name;
#
# To review your project version stats:
#
# make stats;
#
# =================================================================================
#
# Finally, DO NOT MIX TARGETS with: push-major, push-minor or build-name
# While you can do this, it will not produce the obvious result, due to the
# way GNU Make and alike expand variables at make file initialization.
#
# There is no way to exit a makefile early without an error which would be a
# false positive, so.. That's the rest of the story...
#
# =================================================================================

BUILD_STATS ?= project.ver

ifeq (, $(BUILD_STATS))
    void != $(error MakeStats Database name is zero-length)
endif

ifeq (TRUE, $(shell test -d $(BUILD_STATS) && echo TRUE))
    void != $(error MakeStats Database name is a directory)
endif

ifndef BUILD_VERSION_SOURCES
    void != $(error You must define your BUILD_VERSION_SOURCES)
endif

void != if ! test -e $(BUILD_STATS); then \
	printf "%s\n\n" "Creating build statistics database ..." >&2; \
	echo 0 0 0 0 `date +%s` $(USER) `basename $(shell pwd)` > $(BUILD_STATS); \
	touch -mc $(BUILD_VERSION_SOURCES); \
fi;

ifeq (1, $(shell expr `wc -w $(BUILD_STATS) | cut -d ' ' -f1` "<" 7))
    void != $(error Unrecognized MakeStats Database format: $(BUILD_STATS))
endif

MAKESTATS != cat $(BUILD_STATS)

BUILD_MAJOR = $(word 1, $(MAKESTATS))
BUILD_MINOR = $(word 2, $(MAKESTATS))
BUILD_REVISION != expr $(word 3, $(MAKESTATS)) + 1
BUILD_NUMBER != expr $(word 4, $(MAKESTATS)) + 1
BUILD_DATE != date +%s
BUILD_USER  ?= $(USER)
BUILD_NAME = $(wordlist 7, $(words $(MAKESTATS)), $(MAKESTATS))
BUILD_TRIPLET = $(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION)

stats:
	@(  \
	    set -- `cat $(BUILD_STATS)`; \
	    printf "%s\n" \
	    "Build Developer: $$6" \
	    "  Build Version: $$1.$$2.$$3" \
	    "   Build Number: $$4" \
	    "     Build Date: `date --date=@$$5`" \
	    "     Build Name: $${@:7}" \
	);
	@echo

# Update build major, clearing build minor and build revision
push-major:
	@sh -c 'echo `expr $$1 + 1` 0 0 $${@:4} > $(BUILD_STATS);' -- `cat $(BUILD_STATS)`

# Update build minor, clearing build revision
push-minor:
	@sh -c 'echo $$1 `expr $$2 + 1` 0 $${@:4} > $(BUILD_STATS);' -- `cat $(BUILD_STATS)`

# Update build name
build-name:
	@sh -c 'echo $${@:1:6} `read -ep "Enter product or code name: " NAME; echo $$NAME` > $(BUILD_STATS);' -- `cat $(BUILD_STATS)`

push-version: $(BUILD_STATS)

# Update build number; possibly revision, date, and user
push-build: push-version
	@sh -c 'echo $${@:1:3} $(BUILD_NUMBER) $${@:5} > $(BUILD_STATS);' -- `cat $(BUILD_STATS)`

# Update revision, date, and user if sources are newer than stats
$(BUILD_STATS): $(BUILD_VERSION_SOURCES)
	@sh -c 'echo $$1 $$2 $(BUILD_REVISION) $$4 $(BUILD_DATE) $(BUILD_USER) $${@:7} > $(BUILD_STATS);' -- `cat $(BUILD_STATS)`

# These targets will build regardless of existing files
.PHONY: stats build-name push-major push-minor push-version push-build