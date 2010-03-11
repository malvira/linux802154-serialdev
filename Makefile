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

submodule: 
	git submodule update --init
	$(MAKE)

-include $(MC1322X)/Makefile.include






