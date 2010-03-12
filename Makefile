MC1322X := libmc1322x

# all off the common objects for each target
# a COBJ is made for EACH board and goes the obj_$(BOARD)_board directory
# board specific code is OK in these files
COBJS := 

# all of the target programs to build
TARGETS := 

# these targets are built with space reserved for variables needed by ROM services
# this space is initialized with a rom call to rom_data_init
TARGETS_WITH_ROM_VARS := linux

##################################################
# you shouldn't need to edit anything below here #
##################################################

# This Makefile includes the default rule at the top
# it needs to be included first
-include $(MC1322X)/Makefile.include

# this rule will become the default_goal if
# $(MC1322X)/Makefile.include doesn't exist it will try to update the
# submodule, check if $(MC1322X) exists, and if it does
# try make again
submodule: 
	git submodule update --init
	if [ ! -d $(MC1322X) ] ; then echo "*** cannot find MC1322X directory $(MC1322X)" ; exit 2; fi
	$(MAKE)
