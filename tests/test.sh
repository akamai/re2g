#!/bin/sh

re2g=$1;

function re2expect () {
  e=$1;
  shift;
  o=$($re2g "$@")
  if [ "$e" eq "$o" ]; then
    echo SUCCESS "$@" => "'$e'";
  else
    echo FAILURE "$@" => "'$o'" NOT "'$e'";
  fi
}


#$re2g -h;

$re2g fred f; #fred

$re2g fred b; #EMPTY

$re2g -o fred r.; #re

$re2g -v fred r.; #EMPTY

$re2g -vo fred r.; #EMPTY
 

$re2g fred r x; #fxed

$re2g -p fred q z; #fred

$re2g -v fred q z; #fred

$re2g fred q z; #EMPTY

$re2g fredrob r. x; #fxdrob

$re2g -g fredrob r. x; #fxdxb

$re2g -g fredrob 'r(.)' '\1x'; #fexdoxb

