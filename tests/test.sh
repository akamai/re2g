#!/bin/bash

re2g=$1;

fail=0;

function re2expect () {
  e="$1";
  t="$2";
  shift 2;

  o=$(printf -- "$t"|$re2g "$@" /dev/stdin);
  s=$?;


  if [ $s = 0 ]; then 
    if [ "$e" = "$o" ]; then
      echo SUCCESS "$@ <<<\"$t\"" '=>' "'$e'";
    else
      echo FAILURE "$@ <<<\"$t\"" '=>' "'$o'" NOT "'$e'";
      fail=`expr $fail + 1`;
    fi
  else
    echo FAILURE "$@ <<<\"$t\"" ' ERR ' $s; 
    fail=`expr $fail + 1`;
  fi
}


if [ $($re2g -h|md5) = 6e54dc81aa6b720287b09369f7287c5e ]; then
  echo SUCCESS "-h => USAGE";
else
  echo FAILURE "-h => help has diverged"
  fail=1;
fi 

re2expect fred fred f 

re2expect "" fred b 

re2expect re fred -o r.  

re2expect "" fred -v r.  

re2expect "" fred -vo r.  
 

re2expect fxed fred r x 

re2expect fred fred -p q z  

re2expect fred fred -v q z  

re2expect "" fred q z 

re2expect fxdrob fredrob r. x 

re2expect fxdxb fredrob -g r. x  

re2expect fexdoxb fredrob -g 'r(.)' '\1x'  


# NONE

re2expect "" theteststring q
re2expect "theteststring" theteststring t
re2expect "theteststring" theteststring '(t.)'
re2expect "" theteststring '(q.)'
re2expect "hzeteststring" theteststring 't(.)' '\1z'
re2expect "" theteststring 'q(.)' '\1z'

# v

re2expect "theteststring" theteststring -v  q 
re2expect "" theteststring -v t 
re2expect "" theteststring -v '(t.)' 
re2expect "theteststring" theteststring -v '(q.)' 
re2expect "" theteststring -v 't(.)' '\1z' 
re2expect "theteststring" theteststring -v 'q(.)' '\1z' 

# o

re2expect "" theteststring -o q 
re2expect "t" theteststring -o t 
re2expect "th" theteststring -o '(t.)' 
re2expect "" theteststring -o '(q.)' 
re2expect "hz" theteststring -o 't(.)' '\1z' 
re2expect "" theteststring -o 'q(.)' '\1z' 

# p

re2expect "" theteststring -p q 
re2expect "theteststring" theteststring -p t 
re2expect "theteststring" theteststring -p '(t.)' 
re2expect "" theteststring -p '(q.)' 
re2expect "hzeteststring" theteststring -p 't(.)' '\1z' 
re2expect "theteststring" theteststring -p 'q(.)' '\1z' 

# g

re2expect "" theteststring -g q 
re2expect "theteststring" theteststring -g t 
re2expect "theteststring" theteststring -g '(t.)' 
re2expect "" theteststring -g '(q.)' 
re2expect "hzeezsszrzing" theteststring -g 't(.)' '\1z' 
re2expect "" theteststring -g 'q(.)' '\1z' 

# vo

re2expect "theteststring" theteststring -vo q 
re2expect "" theteststring -vo t 
re2expect "" theteststring -vo '(t.)' 
re2expect "theteststring" theteststring -vo '(q.)' 
re2expect "" theteststring -vo 't(.)' '\1z' 
re2expect "theteststring" theteststring -vo 'q(.)' '\1z' 

# vp

re2expect "theteststring" theteststring -vp q 
re2expect "" theteststring -vp t 
re2expect "" theteststring -vp '(t.)' 
re2expect "theteststring" theteststring -vp '(q.)' 
re2expect "theteststring" theteststring -vp 't(.)' '\1z' 
re2expect "theteststring" theteststring -vp 'q(.)' '\1z' 

# vg

re2expect "theteststring" theteststring -vg  q 
re2expect "" theteststring -vg t 
re2expect "" theteststring -vg '(t.)' 
re2expect "theteststring" theteststring -vg '(q.)' 
re2expect "" theteststring -vg 't(.)' '\1z' 
re2expect "theteststring" theteststring -vg 'q(.)' '\1z' 

# op

re2expect "" theteststring -op q 
re2expect "t" theteststring -op t 
re2expect "th" theteststring -op '(t.)' 
re2expect "" theteststring -op '(q.)' 
re2expect "hz" theteststring -op 't(.)' '\1z' 
re2expect "theteststring" theteststring -op 'q(.)' '\1z' 

# og

re2expect "" theteststring -og q 
re2expect "tttt" theteststring -og t 
re2expect "thtetstr" theteststring -og '(t.)' 
re2expect "" theteststring -og '(q.)' 
re2expect "hzezszrz" theteststring -og 't(.)' '\1z' 
re2expect "" theteststring -og 'q(.)' '\1z' 

# pg

re2expect "" theteststring -pg q 
re2expect "theteststring" theteststring -pg t 
re2expect "theteststring" theteststring -pg '(t.)' 
re2expect "" theteststring -pg '(q.)' 
re2expect "hzeezsszrzing" theteststring -pg 't(.)' '\1z' 
re2expect "theteststring" theteststring -pg 'q(.)' '\1z' 

# vop

re2expect "theteststring" theteststring -vop q 
re2expect "" theteststring -vop t 
re2expect "" theteststring -vop '(t.)' 
re2expect "theteststring" theteststring -vop '(q.)' 
re2expect "theteststring" theteststring -vop 't(.)' '\1z' 
re2expect "theteststring" theteststring -vop 'q(.)' '\1z' 

# vog

re2expect "theteststring" theteststring -vog q 
re2expect "" theteststring -vog t 
re2expect "" theteststring -vog '(t.)' 
re2expect "theteststring" theteststring -vog '(q.)' 
re2expect "" theteststring -vog 't(.)' '\1z' 
re2expect "theteststring" theteststring -vog 'q(.)' '\1z' 


# vpg

re2expect "theteststring" theteststring -vpg q 
re2expect "" theteststring -vpg t 
re2expect "" theteststring -vpg '(t.)' 
re2expect "theteststring" theteststring -vpg '(q.)' 
re2expect "theteststring" theteststring -vpg 't(.)' '\1z' 
re2expect "theteststring" theteststring -vpg 'q(.)' '\1z' 

# opg

re2expect "" theteststring -opg q 
re2expect "tttt" theteststring -opg t 
re2expect "thtetstr" theteststring -opg '(t.)' 
re2expect "" theteststring -opg '(q.)' 
re2expect "hzezszrz" theteststring -opg 't(.)' '\1z' 
re2expect "theteststring" theteststring -opg 'q(.)' '\1z' 

# vopg

re2expect "theteststring" theteststring -vopg q 
re2expect "" theteststring -vopg t 
re2expect "" theteststring -vopg '(t.)' 
re2expect "theteststring" theteststring -vopg '(q.)' 
re2expect "theteststring" theteststring -vopg 't(.)' '\1z' 
re2expect "theteststring" theteststring -vopg 'q(.)' '\1z' 

diff  <(grep q tests/test.sh)  <($re2g q tests/test.sh) || fail=$(expr 1 + $fail);

diff  <(grep -v q tests/test.sh)  <($re2g -v q tests/test.sh) || fail=$(expr 1 + $fail);


if [ $fail -gt 0 ]; then
  echo $fail Errors
  exit 1;
fi
