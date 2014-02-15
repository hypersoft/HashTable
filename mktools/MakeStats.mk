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

# Usage ========================================================================
#
# To use this file properly, you must include the file at the end
# of your make script, or at least after the default rule.
#
# Variable: BUILD_VERSION_SOURCES
#
# Populate this variable with all sources that will cause version to change
# when modified.
#
# ==============================================================================
# Do this BEFORE compiling, archiving, and linking your "main object".
# ...
# @$(make-build-number)
# ...
# To increment the build number. 
# ==============================================================================
# Do this AFTER SUCCESSFUL compile of your "main object".
# ...
# @$(make-build-revision)
# ...
# To increment the build revision. 
# ==============================================================================
#
# To increment the minor build version, from the command line you shall issue:
#
# make build-minor;
#
# Which will handle the versioning for you.
#
# Likewise to increment the major build version, from the command line:
#
# make build-major;
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
# Finally, DO NOT MIX TARGETS with: build-major, build-minor or build-name
# While you can do this, it will not produce the obvious result, due to the
# way GNU Make and alike expand variables at make file initialization.
#
# There is no way to exit a makefile early without an error which would be a
# false positive, so.. That's the rest of the story...
#
# =================================================================================

# DB FMT: MAJOR MINOR REVISION BUILD DATE USER PRODUCT...\n
# MAJOR MINOR REVISION and BUILD must be INTEGERS
# DATE must be an EPOCH interval
# USER may not contain spaces

# You can set this variable in YOUR makefile/command-line if need be.
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

ifeq (, $(BUILD_VERSION_SOURCES))
    void != $(error BUILD_VERSION_SOURCES are zero-length)
endif

BUILD_VERSION_SOURCE_CHECK := $(foreach file,$(BUILD_SOURCES),$(shell test -e $(file) || echo $(file)))
BUILD_VERSION_UPDATE_CHECK := $(foreach file,$(BUILD_SOURCES),$(shell test $(file) -n $(BUILD_STATS) && echo $(file)))

ifneq (, $(BUILD_VERSION_SOURCE_CHECK))
    void != $(error Could not find file $(word 1, $(BUILD_VERSION_SOURCE_CHECK)) specified by BUILD_VERSION_SOURCES)
endif

void != if ! test -e $(BUILD_STATS); then \
	printf "%s\n\n" "Creating build statistics database ..." >&2; \
	echo 0 0 0 0 `date +%s` $(USER) "`basename $(shell pwd)`" > $(BUILD_STATS); \
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

BUILD_UPDATES:=$(strip $(foreach file,$(BUILD_VERSION_SOURCES),$(shell test $(file) -nt $(BUILD_STATS) && echo $(file))))

# Update revision, date, and user if sources are newer than stats
make-build-revision = \
sh -c ' \
    if [ -n "$(BUILD_UPDATES)" ] && [ "$(BUILD_REVISION)" != "$$3" ]; then \
	echo $$1 $$2 $(BUILD_REVISION) $(BUILD_NUMBER) $(BUILD_DATE) "$(BUILD_USER)" "$${@:7}" > $(BUILD_STATS); \
    fi; echo -n;' -- \
`cat $(BUILD_STATS)`

# Update build number if they don't match
make-build-number = \
sh -c ' \
    if [ "$(BUILD_NUMBER)" != "$$4" ]; then \
	echo $${@:1:3} $(BUILD_NUMBER) "$${@:5}" > $(BUILD_STATS); \
    fi; echo -n;' -- \
`cat $(BUILD_STATS)`

build-stats:
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

# Update build name
build-name:
	@sh -c 'echo $${@:1:6} `read -ep "Enter product or code name: " NAME; echo $$NAME` > $(BUILD_STATS);' -- `cat $(BUILD_STATS)`

# Update build major, clearing build minor and build revision
build-major:
	@sh -c 'echo `expr $$1 + 1` 0 0 "$${@:4}" > $(BUILD_STATS);' -- `cat $(BUILD_STATS)`

# Update build minor, clearing build revision
build-minor:
	@sh -c 'echo $$1 `expr $$2 + 1` 0 "$${@:4}" > $(BUILD_STATS);' -- `cat $(BUILD_STATS)`

