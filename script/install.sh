#! /bin/bash
######################################33

#
# Pull in project CmdParameter and build
# 
cd ..
if [ ! -d "CmdParameter" ]; then
	git clone https://github.com/wimrijnders/CmdParameter.git
fi

cd CmdParameter
git checkout master
git pull origin master
make clean
make DEBUG=1 all
make all
