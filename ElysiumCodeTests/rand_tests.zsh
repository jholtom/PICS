#!/usr/bin/env zsh

a=(uart cmd reg)

while ;
do
  val=$a[$(( ${RANDOM}%${#a[@]}+1 ))]
  dir=${val:u}Sandbox
  cd $dir
  python -m run_tests ${val}-test-suite || break
  cd ..
  sleep 5
done
