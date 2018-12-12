#!/usr/bin/env zsh

a=(uart-test-suite.py cmd-test-suite.py reg-test-suite.py random-homer.py)

while ;
do
  val=$a[$(( ${RANDOM}%${#a[@]}+1 ))]
  ./${val} || break
  sleep 5
done
