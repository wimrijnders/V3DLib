#
# Before using make, you need to create the file dependencies:
#
# > script/gen.sh
#
# There are four builds possible, with output directories:
#
#   obj/emu          - using emulator
#   obj/emu-debug    - output debug info, using emulator
#   obj/qpu          - using hardware
#   objiqpu-debug    - output debug info, using hardware
#
#==============================================================================
# NOTES
# =====
#
# * valgrind goes into a fit when used on `runTests`:
# 
#    --8346-- WARNING: Serious error when reading debug info
#    --8346-- When reading debug info from /lib/arm-linux-gnueabihf/ld-2.28.so:
#    --8346-- Ignoring non-Dwarf2/3/4 block in .debug_info
#    --8346-- WARNING: Serious error when reading debug info
#    --8346-- When reading debug info from /lib/arm-linux-gnueabihf/ld-2.28.so:
#    --8346-- Last block truncated in .debug_info; ignoring
#    ==8346== Conditional jump or move depends on uninitialised value(s)
#    ==8346==    at 0x401A5D0: index (in /lib/arm-linux-gnueabihf/ld-2.28.so)
#    ...
#
#  This has to do with valgrind not being able to read compressed debug info.
#  Note that this happens in system libraries.
#
#  Tried countering this with compiler options:
#
#  * -Wa,--nocompress-debug-sections -Wl,--compress-debug-sections=none
#  * -gz=none
#
#  ...with no effect. This is somewhat logical, the system libraries are
#  pre-compiled.
#
###############################################################################

#
# Stuff for external libraries
#
INCLUDE_EXTERN= \
 -I ../CmdParameter/Lib \
 -I mesa/include \
 -I mesa/src

LIB_EXTERN= \
 -Lobj/mesa/bin -lmesa

LIB_DEPEND=

ifeq ($(DEBUG), 1)
	LIB_EXTERN += -L ../CmdParameter/obj-debug -lCmdParameter
	LIB_DEPEND += ../CmdParameter/obj-debug/libCmdParameter.a
else
	LIB_EXTERN += -L ../CmdParameter/obj -lCmdParameter
	LIB_DEPEND += ../CmdParameter/obj/libCmdParameter.a
endif


ROOT= Lib

# Compiler and default flags
CXX= g++
LINK= $(CXX) $(CXX_FLAGS)

LIBS := $(LIB_EXTERN)

#
# -I is for access to bcm functionality
#
# -Wno-psabi avoids following note (happens in unit tests):
#
#    note: parameter passing for argument of type ‘std::move_iterator<Catch::SectionEndInfo*>’ changed in GCC 7.1
#
#    It is benign: https://stackoverflow.com/a/48149400 
#
CXX_FLAGS = \
 -Wall \
 -Wconversion \
 -Wno-psabi \
 -I $(ROOT) $(INCLUDE_EXTERN) -MMD -MP -MF"$(@:%.o=%.d)"

# Object directory
OBJ_DIR := obj


# QPU or emulation mode
ifeq ($(QPU), 1)
$(info Building for QPU)

# Check platform before building.
# Can't be indented, otherwise make complains.
RET := $(shell Tools/detectPlatform.sh 1>/dev/null && echo "yes" || echo "no")
#$(info  info: '$(RET)')
ifneq ($(RET), yes)
$(error QPU-mode specified on a non-Pi platform; aborting)
else
$(info Building on a Pi platform)
endif

  CXX_FLAGS += -DQPU_MODE -I /opt/vc/include
  OBJ_DIR := $(OBJ_DIR)/qpu
	LIBS += -L /opt/vc/lib -l bcm_host
else
  OBJ_DIR := $(OBJ_DIR)/emu
  CXX_FLAGS += -DEMULATION_MODE
endif

# Debug mode
ifeq ($(DEBUG), 1)
  CXX_FLAGS += -DDEBUG -g
  OBJ_DIR := $(OBJ_DIR)-debug
else
  # -DNDEBUG	disables assertions
  # -g0 still adds debug info, using -s instead
  CXX_FLAGS += -DNDEBUG -s
endif

-include sources.mk

LIB = $(patsubst %,$(OBJ_DIR)/Lib/%,$(OBJ))

EXAMPLE_TARGETS = $(patsubst %,$(OBJ_DIR)/bin/%,$(EXAMPLES))
TESTS_OBJ = $(patsubst %,$(OBJ_DIR)/%,$(TESTS_FILES))
#$(info $(TESTS_OBJ))


# Example object files
EXAMPLES_OBJ = $(patsubst %,$(OBJ_DIR)/%,$(EXAMPLES_EXTRA))
#$(info $(EXAMPLES_OBJ))

# Dependencies from list of object files
DEPS := $(LIB:.o=.d)
-include $(DEPS)

# Dependencies for the include files in the Examples directory.
# Basically, every .h file under examples has a .d in the build directory
EXAMPLES_DEPS = $(EXAMPLES_OBJ:.o=.d)
-include $(EXAMPLES_DEPS)

V3DLIB=$(OBJ_DIR)/libv3dlib.a
MESA_LIB = obj/mesa/bin/libmesa.a


# Top-level targets

.PHONY: help clean all lib test $(EXAMPLES) init

# Following prevents deletion of object files after linking
# Otherwise, deletion happens for targets of the form '%.o'
.PRECIOUS: $(OBJ_DIR)/%.o


help:
	@echo 'Usage:'
	@echo
	@echo '    make [QPU=1] [DEBUG=1] [target]*'
	@echo
	@echo 'Where target:'
	@echo
	@echo '    help          - Show this text'
	@echo '    all           - Build all test programs'
	@echo '    clean         - Delete all interim and target files'
	@echo '    test          - Run the unit tests'
	@echo
	@echo '    one of the test programs - $(EXAMPLES)'
	@echo
	@echo 'Flags:'
	@echo
	@echo '    QPU=1         - Output code for hardware. If not specified, the code is compiled for the emulator'
	@echo '    DEBUG=1       - If specified, the source code and target code is shown on stdout when running a test'
	@echo

all: $(V3DLIB) $(EXAMPLES)

clean:
	rm -rf obj/emu obj/emu-debug obj/qpu obj/qpu-debug obj/test

init:
	@./script/detect_tabs.sh
	@mkdir -p ./obj/test


#
# Targets for static library
#

$(V3DLIB): $(LIB) $(MESA_LIB)
	@echo Creating $@
	@ar rcs $@ $^

$(MESA_LIB):
	cd mesa && make compile


# Rule for creating object files
$(OBJ_DIR)/%.o: %.cpp | init
	@echo Compiling $<
	@mkdir -p $(@D)
	@$(CXX) -std=c++11 -c -o $@ $< $(CXX_FLAGS)

# Same thing for C-files
$(OBJ_DIR)/%.o: %.c | init
	@echo Compiling $<
	@mkdir -p $(@D)
	@$(CXX) -x c -c -o $@ $< $(CXX_FLAGS)


$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Examples/%.o $(EXAMPLES_OBJ) $(V3DLIB) $(LIB_DEPEND)
	@echo Linking $@...
	@mkdir -p $(@D)
	@$(LINK) $^ $(LIBS) -o $@

$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Tools/%.o $(V3DLIB) $(LIB_DEPEND)
	@echo Linking $@...
	@mkdir -p $(@D)
	@$(LINK) $^ $(LIBS) -o $@


$(EXAMPLES) :% : $(OBJ_DIR)/bin/%


#
# Targets for Unit Tests
#

UNIT_TESTS := $(OBJ_DIR)/bin/runTests

## sudo required for QPU-mode on Pi
ifeq ($(QPU), 1)
	SUDO := sudo
else
	SUDO :=
endif


$(UNIT_TESTS): $(TESTS_OBJ) $(EXAMPLES_OBJ) $(V3DLIB) $(LIB_DEPEND)
	@echo Linking unit tests
	@mkdir -p $(@D)
	@$(CXX) $(CXX_FLAGS) $(TESTS_OBJ) $(EXAMPLES_OBJ) -L$(OBJ_DIR) -lv3dlib $(LIBS) -o $@

runTests: $(UNIT_TESTS)


make_test: runTests ID Hello Rot3D ReqRecv GCD Tri detectPlatform OET

test : make_test
	@echo Running unit tests with \'$(SUDO) $(UNIT_TESTS)\'
	@$(SUDO) $(UNIT_TESTS)


###############################
# Gen stuff
################################

gen : $(OBJ_DIR)/sources.mk

$(OBJ_DIR)/sources.mk :
	@mkdir -p $(@D)
	@script/gen.sh
