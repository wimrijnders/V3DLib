#! /bin/bash
######################################33

BASE=`pwd`

#
# Pull in project CmdParameter and build
# 
cd ..
if [ -d "CmdParameter" ]; then
	git clone https://github.com/wimrijnders/CmdParameter.git
fi

cd $BASE
cd ../CmdParameter
git checkout master
git pull origin master
make all
