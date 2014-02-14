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

ifneq (FALSE, $(NOTICE))
NOTICE := $(shell { \
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
# To trigger updating of stats, use push-build as a final prerequisite:
#
# my_program: some.c some.h push-build
#
# DO NOTE: Each pre-requisite for push-build WILL increment build number!
# so only call for it once in the project build process.
#
# Which will automatically increment your build revision number.
#
# To increment the minor build version, from the command line you shall issue:
#
# make push-minor;
#
# Which will handle the versioning for you.
#
# Likewise to increment the major build version, from the command line:
#
# make push-major
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

BUILD_STATS = project.ver

MAKESTATS != if ! test -e $(BUILD_STATS); then \
	printf "%s\n\n" "Creating build statistics database ..." >&2; \
	echo 0 0 1 1 `date +%s` $(USER) `basename $(shell pwd)` > $(BUILD_STATS); \
	touch $(BUILD_VERSION_SOURCES); \
fi;

BUILD_DUMP_STATS = echo $(BUILD_MAJOR) $(BUILD_MINOR) $(BUILD_REVISION) \
    $(BUILD_NUMBER) $(BUILD_DATE) $(USER) $(BUILD_NAME) > $(BUILD_STATS)

MAKESTATS != cat $(BUILD_STATS) 2>&- || true

BUILD_MAJOR = $(word 1, $(MAKESTATS))
BUILD_MINOR = $(word 2, $(MAKESTATS))
BUILD_REVISION = $(word 3, $(MAKESTATS))
BUILD_NUMBER = $(word 4, $(MAKESTATS))
BUILD_DATE = $(word 5, $(MAKESTATS))
BUILD_USER  = $(word 6, $(MAKESTATS))
BUILD_NAME = $(wordlist 7, $(words $(MAKESTATS)), $(MAKESTATS))
BUILD_TRIPLET = $(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION)


stats: BUILD_REVISION != expr $(BUILD_REVISION) - 1
stats: BUILD_NUMBER != expr $(BUILD_NUMBER) - 1
stats:
	@echo Build Developer: $(BUILD_USER)
	@echo '  'Build Version: $(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION)
	@echo '   'Build Number: $(BUILD_NUMBER)
	@echo '     'Build Date: `date --date=@$(BUILD_DATE)`
	@echo '     'Build Name: $(BUILD_NAME)
	@echo

push-major: BUILD_MAJOR != expr $(BUILD_MAJOR) + 1
push-major: BUILD_MINOR = 0
push-major: BUILD_REVISON = 0
push-major:
	@$(BUILD_DUMP_STATS);
	@echo
	
push-minor: BUILD_MINOR != expr $(BUILD_MINOR) + 1
push-minor: BUILD_REVISON = 0
push-minor:
	@$(BUILD_DUMP_STATS);
	@echo

build-name:
	@$(shell read -ep "Enter product or code name: " NAME; echo -n \
		$(BUILD_MAJOR) $(BUILD_MINOR) $(BUILD_REVISION) $(BUILD_NUMBER)  \
		$(BUILD_DATE) $(USER) $$NAME > $(BUILD_STATS); \
	)

$(BUILD_STATS): BUILD_REVISION != expr $(BUILD_REVISION) + 1
$(BUILD_STATS): BUILD_DATE != date +%s
$(BUILD_STATS): $(BUILD_VERSION_SOURCES)
	@$(BUILD_DUMP_STATS)

push-build: $(BUILD_STATS)
	@(set -- `cat $(BUILD_STATS)`; echo $${@:1:3} `expr $$4 + 1` $${@:5} > $(BUILD_STATS));
