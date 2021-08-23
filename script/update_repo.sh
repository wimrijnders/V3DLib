#! /bin/bash
  
function get_version() {
  cur_version=`grep Version ../$repo/README.md`
  cur_version=${cur_version//Version /}
}


function update_repo() {
  local repo="$1"
  local repo_path="$2"
  local expected_version="$3"

  #
  # Check version WebGen
  #
  get_version
  if [ "$expected_version" == "$cur_version" ]; then
  #  echo "Versions match"
    exit 0  # all is well
  else
    echo $repo current version: $cur_version
  fi

  echo "Project $repo not present or version mismatch."
  echo "Updating $repo..."

  #
  # Pull in project
  # 
  cd ..   # This assumes you are calling from top-level project dir, eg. 'script/install.sh'

  if [ ! -d "$repo" ]; then
    git clone $repo_path
  fi

  cd $repo
  git checkout master
  git pull origin master

  # Sanity check
  get_version
  if [ "$expected_version" != "$cur_version" ]; then
    echo
    echo "ERROR: WebGen versions do not match! current: $cur_version, expected: $expected_version"
    echo "  ... perhaps you need to update the expected version number?"
    echo
    exit 1
  fi
}

