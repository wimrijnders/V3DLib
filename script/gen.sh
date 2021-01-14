#!/bin/bash
#
# Pre: create obj directories beforehand
#
# ------------------------------------------------------------------------------
# NOTES
# =====
#
# * Here-delimiter with quotes does not do parameter/command substitution
#   e.g.:
#
#    cat <<'END'
#    Generated on: $(date)
#    END
#
# ------------------------------------------------------------------------------
# NOTES
# =====
#
# - Consider using make commands as here: https://stackoverflow.com/a/32699423
#
################################################################################

# Make list of Library object files
CPP_FILES=$(find Lib -name '*.cpp')
OBJ_CPP=$(echo "$CPP_FILES" | sed "s/\\.cpp$/\\.o  \\\\/g")

C_FILES=$(find Lib -name '*.c')
OBJ_C=$(echo "$C_FILES" | sed "s/\\.c$/\\.o  \\\\/g")

OBJ_TMP=$(echo "$OBJ_CPP
$OBJ_C
")
OBJ=$(echo "$OBJ_TMP" | sed "s/^Lib\\//  /g")
#echo $OBJ


TEST_FILES=$(find Tests -name '*.cpp')
CPP_OBJ_TEST_TMP=$(echo "$TEST_FILES" | sed "s/\\.cpp$/\\.o  \\\\/g")

C_TEST_FILES=$(find Tests -name '*.c')
C_OBJ_TEST_TMP=$(echo "$C_TEST_FILES" | sed "s/\\.c$/\\.o  \\\\/g")

OBJ_TEST_TMP=$(echo "$CPP_OBJ_TEST_TMP
$C_OBJ_TEST_TMP
")
OBJ_TEST=$(echo "$OBJ_TEST_TMP" | sed "s/^/  /g")

EXAMPLES_SUPPORT="$(find Examples/Rot3DLib -name '*.cpp')
$(find Examples/Support -name '*.cpp')
"
EXAMPLES_EXTRA=$(echo "$EXAMPLES_SUPPORT" | sed "s/\\.cpp$/\\.o  \\\\/g" | sed "s/^/  /")

# Get list of executables
# NOTE: grepping on 'main(' is not fool-proof, of course.
EXE1=$(grep -rl "main(" Examples/ Tools/)
EXE2=$(echo "$EXE1" | sed "s/\\.cpp$/  \\\\/g")
EXAMPLES=$(echo "$EXE2" | sed "s/^.*\//  /g")

#
# NOT WORKING YET
#
## Get list of directories to create in the obj dir
## Note that 'old' is excluded
#OBJ_DIRS1=$(find . -name *cpp | xargs dirname | sort | uniq | grep -v old | sed "s/^\.\//\$(OBJ_DIR)\//")
#OBJ_DIRS=$(echo "$OBJ_DIRS1" | sed "s/^/	@mkdir -p /g")

mkdir -p obj

cat << END > obj/sources.mk
# Generated on: $(date)
#
######################################

# Library Object files - only used for LIB
OBJ := \\
$OBJ

# All programs in the Examples *and Tools* directory
EXAMPLES := \\
$EXAMPLES

# support files for examples
EXAMPLES_EXTRA := \\
$EXAMPLES_EXTRA
# support files for tests
TESTS_FILES := \\
$OBJ_TEST

END
