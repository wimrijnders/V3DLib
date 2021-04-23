#! /bin/bash
######################################33

# Expected version of CmdParameter
expected_version=0.4.1

function get_version() {
  cur_version=`grep Version ../CmdParameter/README.md`
  cur_version=${cur_version//Version /}
}

#
# Check version CmdParameter
#
get_version
if [ "$expected_version" == "$cur_version" ]; then
#  echo "Versions match"
  exit 0  # all is well
else
  echo CmdParameter current version: $cur_version
fi

echo "Library CmdParameters not present or version mismatch."
echo "Updating library CmdParameters..."

#
# Pull in project CmdParameter
# 
cd ..

if [ ! -d "CmdParameter" ]; then
	git clone https://github.com/wimrijnders/CmdParameter.git
fi

cd CmdParameter
git checkout master
git pull origin master

# Sanity check
get_version
if [ "$expected_version" != "$cur_version" ]; then
  echo
  echo "ERROR: CmdParameter versions do not match! current: $cur_version, expected: $expected_version"
  echo "  ... perhaps you need to update the expected version number?"
  echo
  exit 1
fi

#
# Always build from scratch
# 
make clean
make DEBUG=1 all
make all
