#! /bin/bash
###############################################################################
. "script/update_repo.sh"

update_repo CmdParameter "https://github.com/wimrijnders/CmdParameter.git" 0.4.1

#
# Always build from scratch
# 
#make clean
#make DEBUG=1 all
#make DEBUG=0 all
#make all
