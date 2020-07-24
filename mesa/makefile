#
# To get all external includes under dir `drivers` with path `include`
#
#     grep -r "#include <linux/.*>" drivers | sed "s/^.*:#include </include\//" | sed "s/>.*$//" | sort | uniq | xargs git checkout master --
#
# This assumes -I./include
#
################################################################################
#SRCDIR = arch/arm/include drivers 
#SRCDIR = drivers/gpu/drm
SRCDIR = src
OBJDIR = ../obj/mesa/src
BINDIR = ../obj/mesa/bin
MAINDIR = ../obj/mesa/main

XX_FLAGS = -MMD -MP -MF"$(@:%.o=%.d)"

INCLUDES = \
-I ./src/drm/ \
-I ./include/ \
-I ./include/drm-uapi \
-I ./src/ \
-I ./src/compiler/ \
-I ./src/mesa/ \
-I ./src/broadcom/ \
-I ./src/util/ \
-I ./src/gallium/include/ \
-I ./src/gallium/auxiliary/

# expat is an XML parser library
#/usr/lib/arm-linux-gnueabihf/libexpat.so
LIBS = \
-l mesa \
-l pthread \
-l z \
-l expat


CFLAGS = -g -Wall -O3 -pthread \
-D_GNU_SOURCE \
-D MAJOR_IN_SYSMACROS \
-D HAVE_PTHREAD \
-D HAVE_TIMESPEC_GET \
-D HAVE_ENDIAN_H \
-D V3D_VERSION=33 \
$(INCLUDES) \
$(XX_FLAGS)


#LINK= g++ $(XX_FLAGS)
LINK= g++


# Get Only the Internal Structure of SRCDIR Recursively
STRUCTURE := $(shell find $(SRCDIR) -type d)     
#$(info structure: $(STRUCTURE))

# Get All Files inside the STRUCTURE Variable
CODEFILES := $(addsuffix /*,$(STRUCTURE))
CODEFILES := $(wildcard $(CODEFILES))            

# Filter Only Specific Files                                
SRCFILES := $(filter %.c %.cpp,$(CODEFILES))
#$(info  src: $(SRCFILES))
HDRFILES := $(filter %.h,$(CODEFILES))
OBJFILES := $(subst $(SRCDIR),$(OBJDIR)$(SRC_DIR),$(SRCFILES))
OBJFILES := $(OBJFILES:%.cpp=%.o)
OBJFILES := $(OBJFILES:%.c=%.o)
#OBJFILES := $(OBJDIR)/$(SRCFILES:%.c=%.o)
#$(info  obj: $(OBJFILES))

DEPEND_FILES := $(addprefix $(SRCDIR)/,%.c %.h)
#$(info depend: $(DEPEND_FILES))


#LIB = $(patsubst %,$(OBJDIR)/%,$(OBJFILES))
LIB = $(OBJFILES)
#$(info lib:  $(LIB))

# Dependencies from list of object files
DEPS := $(LIB:.o=.d)
#$(info deps:  $(DEPS))
-include $(DEPS)

MESA_LIB=$(BINDIR)/libmesa.a

###############
# Rules
###############

#compile: $(NIR_INCLUDES) $(OBJFILES)
compile: $(BINDIR)/main

clean:
	rm -rf $(OBJDIR)/*
	rm -rf $(BINDIR)/*
	rm -rf $(MAINDIR)/*


#$(OBJDIR)/%.o: $(subst $(OBJDIR),$(SRCDIR),%.c %.h)
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling $<
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo Compiling $<
	mkdir -p $(@D)
	g++ $(CFLAGS) -c $< -o $@

$(MESA_LIB): $(OBJFILES)
	@echo Creating $@
	mkdir -p $(@D)
	@ar rcs $@ $^


$(MAINDIR)/%.o: main/%.c
	@echo Compiling $<
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/main: $(MAINDIR)/main.o $(MESA_LIB)
	@echo Linking $@...
	$(LINK) -L $(BINDIR) $(LIBS) $^ -o $@
