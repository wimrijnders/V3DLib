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
OBJ1=$(find Lib -name '*.cpp')
OBJ2=$(echo "$OBJ1" | sed "s/\\.cpp$/\\.o  \\\\/g")
#echo ====
OBJ=$(echo "$OBJ2" | sed "s/^Lib\\//  /g")
#echo $OBJ

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


## Rule to create obj tree
#$   (OBJ_DIR):
#	@mkdir -p \$(OBJ_DIR)/bin
#$OBJ_DIRS

END
