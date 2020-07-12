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
#
###############################################################################

#QPU := 1

#
# Stuff for external libraries
#
INCLUDE_EXTERN=-I ../CmdParameter/Lib
LINK_DIR_EXTERN=-L ../CmdParameter/obj
LIB_EXTERN = -l ../CmdParameter/obj/libCmdParameter.a


# Root directory of QPULib repository
ROOT = Lib

# Compiler and default flags
CXX = g++
LINK= $(CXX) $(CXX_FLAGS)

LIBS := $(LINK_DIR_EXTERN)$(LIB_EXTERN)

# -I is for access to bcm functionality
CXX_FLAGS = -Wconversion -std=c++0x -I $(ROOT) $(INCLUDE_EXTERN) -MMD -MP -MF"$(@:%.o=%.d)" -g

# Object directory
OBJ_DIR = obj


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
  CXX_FLAGS += -DDEBUG    # -Wall
  OBJ_DIR := $(OBJ_DIR)-debug
else
  CXX_FLAGS += -DNDEBUG		# Disable assertions
endif

-include obj/sources.mk

LIB = $(patsubst %,$(OBJ_DIR)/%,$(OBJ))

EXAMPLE_TARGETS = $(patsubst %,$(OBJ_DIR)/bin/%,$(EXAMPLES))


# Example object files
EXAMPLES_EXTRA = \
	Rot3DLib/Rot3DKernels.o \
	Support/Settings.o

EXAMPLES_OBJ = $(patsubst %,$(OBJ_DIR)/Examples/%,$(EXAMPLES_EXTRA))
$(info $(EXAMPLES_OBJ))

# Dependencies from list of object files
DEPS := $(LIB:.o=.d)
#$(info $(DEPS))
-include $(DEPS)

# Dependencies for the include files in the Examples directory.
# Basically, every .h file under examples has a .d in the build directory
EXAMPLES_DEPS = $(EXAMPLES_OBJ:.o=.d)
$(info $(EXAMPLES_DEPS))
-include $(EXAMPLES_DEPS)


# Top-level targets

.PHONY: help clean all lib test $(EXAMPLES)

# Following prevents deletion of object files after linking
# Otherwise, deletion happens for targets of the form '%.o'
.PRECIOUS: $(OBJ_DIR)/%.o  \
	$(OBJ_DIR)/Source/%.o    \
	$(OBJ_DIR)/Target/%.o    \
	$(OBJ_DIR)/VideoCore/%.o \
	$(OBJ_DIR)/VideoCore/vc6/%.o \
	$(OBJ_DIR)/Examples/%.o


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

all: $(OBJ_DIR) $(EXAMPLE_TARGETS)

clean:
	rm -rf obj/emu obj/emu-debug obj/qpu obji/qpu-debug


#
# Targets for static library
#

QPU_LIB=$(OBJ_DIR)/libQPULib.a
#$(info LIB: $(LIB))

$(QPU_LIB): $(LIB)
	@echo Creating $@
	@ar rcs $@ $^

$(OBJ_DIR)/%.o: $(ROOT)/%.cpp | $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c -o $@ $< $(CXX_FLAGS)


# Same thing for C-files
$(OBJ_DIR)/%.o: $(ROOT)/%.c | $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c -o $@ $< $(CXX_FLAGS)

#
# Targets for Examples and Tools
#

$(OBJ_DIR)/bin/Rot3DLib: $(OBJ_DIR)/Examples/Rot3DLib/Rot3DKernels.o


$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Examples/Rot3DLib/%.o $(QPU_LIB)
	@echo Linking $@...
	@$(LINK) $^ -o $@

$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Examples/%.o $(QPU_LIB) $(OBJ_DIR)/Examples/Support/Settings.o
	@echo Linking $@...
	@$(LINK) $^ $(LIBS) -o $@

$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Tools/%.o $(QPU_LIB)
	@echo Linking $@...
	@$(LINK) $^ $(LIBS) -o $@

# General compilation of cpp files
# Keep in mind that the % will take into account subdirectories under OBJ_DIR.
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c $(CXX_FLAGS) -o $@ $<

$(EXAMPLES) :% :$(OBJ_DIR)/bin/%


#
# Targets for Unit Tests
#

RUN_TESTS := $(OBJ_DIR)/bin/runTests

# sudo required for QPU-mode on Pi
ifeq ($(QPU), 1)
	RUN_TESTS := sudo $(RUN_TESTS)
endif


# Source files with unit tests to include in compilation
UNIT_TESTS =          \
	Tests/testMain.cpp  \
	Tests/testRot3D.cpp \
	Tests/testDSL.cpp

# For some reason, doing an interim step to .o results in linkage errors (undefined references).
# So this target compiles the source files directly to the executable.
#
# Flag `-Wno-psabi` is to surpress a superfluous warning when compiling with GCC 6.3.0
#
$(OBJ_DIR)/bin/runTests: $(UNIT_TESTS) $(EXAMPLES_OBJ) | $(QPU_LIB)
	@echo Compiling unit tests
	@$(CXX) $(CXX_FLAGS) -Wno-psabi $^ -L$(OBJ_DIR) -lQPULib $(LIBS) -o $@

make_test: $(OBJ_DIR)/bin/runTests

test : | make_test AutoTest
	@echo Running unit tests with '$(RUN_TESTS)'
	@$(RUN_TESTS)

#
# Other targets
#

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)/Source
	@mkdir -p $(OBJ_DIR)/Target
	@mkdir -p $(OBJ_DIR)/VideoCore/vc6
	@mkdir -p $(OBJ_DIR)/Examples/Rot3DLib   # Creates Examples as well
	@mkdir -p $(OBJ_DIR)/Examples/Support
	@mkdir -p $(OBJ_DIR)/Tools
	@mkdir -p $(OBJ_DIR)/bin


###############################
# Gen stuff
###############################

gen : $(OBJ_DIR)/sources.mk

$(OBJ_DIR)/sources.mk : $(OBJ_DIR)
	script/gen.sh
