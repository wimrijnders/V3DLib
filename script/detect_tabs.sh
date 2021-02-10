#!/bin/bash

grep -rl -P '^\t' Lib Examples Tests > /dev/null

if [ $? -eq 0 ]; then
  echo
  echo There are files with tab indentation!
  echo To list the files in question, run: \'grep -rl -P \"^\\t\" Lib Examples Tests\'
  echo
fi
