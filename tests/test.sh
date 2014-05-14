#!/bin/sh

re2g=$1;

fail=0;

function re2expect () {
  e=$1;
  shift;
  o=$($re2g "$@");
  s=$?;
  if [ $s = 0 ]; then 
    if [ "$e" = "$o" ]; then
      echo SUCCESS "$@" '=>' "'$e'";
    else
      echo FAILURE "$@" '=>' "'$o'" NOT "'$e'";
      fail=`expr $fail + 1`;
    fi
  else
    echo FAILURE "$@" ' ERR ' $s; 
    fail=`expr $fail + 1`;
  fi
}


#$re2g -h;

re2expect fred fred f 

re2expect "" fred b 

re2expect re -o fred r. 

re2expect "" -v fred r. 

re2expect "" -vo fred r. 
 

re2expect fxed fred r x 

re2expect fred -p fred q z 

re2expect fred -v fred q z 

re2expect "" fred q z 

re2expect fxdrob fredrob r. x 

re2expect fxdxb -g fredrob r. x 

re2expect fexdoxb -g fredrob 'r(.)' '\1x' 

if [ $fail -gt 0 ]; then
  echo $fail Errors
  exit 1;
fi
