#!/bin/bash

grep -rl -P '^\t' Lib > /dev/null

if [ $? -eq 0 ]; then
  echo
  echo There are still files with tab indentation!
  echo To list the files in question, run: \'grep -rl -P \"^\\t\" Lib\'
  echo
fi
