#!/bin/bash

re2g=$1;

fail=0;

function re2expect () {
  e="$1";
  t="$2";
  shift 2;

  o=$(printf -- "$t"|$re2g "$@");
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


if [ $($re2g -h|md5) = 55898691fbbb3aaee0af6bdb3b2f42e5 ]; then
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
 

re2expect fxed fred -s r x 

re2expect fred fred -sp q z  

re2expect fred fred -sv q z  

re2expect "" fred -s q z 

re2expect fxdrob fredrob -s r. x 

re2expect fxdxb fredrob -sg r. x  

re2expect fexdoxb fredrob -sg 'r(.)' '\1x'  


# NONE

re2expect "" theteststring q
re2expect "theteststring" theteststring t
re2expect "theteststring" theteststring '(t.)'
re2expect "" theteststring '(q.)'
re2expect "hzeteststring" theteststring -s 't(.)' '\1z'
re2expect "" theteststring -s 'q(.)' '\1z'

# v

re2expect "theteststring" theteststring -v  q 
re2expect "" theteststring -v t 
re2expect "" theteststring -v '(t.)' 
re2expect "theteststring" theteststring -v '(q.)' 
re2expect "" theteststring -vs 't(.)' '\1z' 
re2expect "theteststring" theteststring -vs 'q(.)' '\1z' 

# o

re2expect "" theteststring -o q 
re2expect "t" theteststring -o t 
re2expect "th" theteststring -o '(t.)' 
re2expect "" theteststring -o '(q.)' 
re2expect "hz" theteststring -os 't(.)' '\1z' 
re2expect "" theteststring -os 'q(.)' '\1z' 

# p

re2expect "" theteststring -p q 
re2expect "theteststring" theteststring -p t 
re2expect "theteststring" theteststring -p '(t.)' 
re2expect "" theteststring -p '(q.)' 
re2expect "hzeteststring" theteststring -ps 't(.)' '\1z' 
re2expect "theteststring" theteststring -ps 'q(.)' '\1z' 

# g

re2expect "" theteststring -g q 
re2expect "theteststring" theteststring -g t 
re2expect "theteststring" theteststring -g '(t.)' 
re2expect "" theteststring -g '(q.)' 
re2expect "hzeezsszrzing" theteststring -gs 't(.)' '\1z' 
re2expect "" theteststring -gs 'q(.)' '\1z' 

# vo

re2expect "theteststring" theteststring -vo q 
re2expect "" theteststring -vo t 
re2expect "" theteststring -vo '(t.)' 
re2expect "theteststring" theteststring -vo '(q.)' 
re2expect "" theteststring -vos 't(.)' '\1z' 
re2expect "theteststring" theteststring -vos 'q(.)' '\1z' 

# vp

re2expect "theteststring" theteststring -vp q 
re2expect "" theteststring -vp t 
re2expect "" theteststring -vp '(t.)' 
re2expect "theteststring" theteststring -vp '(q.)' 
re2expect "theteststring" theteststring -vps 't(.)' '\1z' 
re2expect "theteststring" theteststring -vps 'q(.)' '\1z' 

# vg

re2expect "theteststring" theteststring -vg  q 
re2expect "" theteststring -vg t 
re2expect "" theteststring -vg '(t.)' 
re2expect "theteststring" theteststring -vg '(q.)' 
re2expect "" theteststring -vgs 't(.)' '\1z' 
re2expect "theteststring" theteststring -vgs 'q(.)' '\1z' 

# op

re2expect "" theteststring -op q 
re2expect "t" theteststring -op t 
re2expect "th" theteststring -op '(t.)' 
re2expect "" theteststring -op '(q.)' 
re2expect "hz" theteststring -ops 't(.)' '\1z' 
re2expect "theteststring" theteststring -ops 'q(.)' '\1z' 

# og

re2expect "" theteststring -og q 
re2expect "tttt" theteststring -og t 
re2expect "thtetstr" theteststring -og '(t.)' 
re2expect "" theteststring -og '(q.)' 
re2expect "hzezszrz" theteststring -ogs 't(.)' '\1z' 
re2expect "" theteststring -ogs 'q(.)' '\1z' 

# pg

re2expect "" theteststring -pg q 
re2expect "theteststring" theteststring -pg t 
re2expect "theteststring" theteststring -pg '(t.)' 
re2expect "" theteststring -pg '(q.)' 
re2expect "hzeezsszrzing" theteststring -pgs 't(.)' '\1z' 
re2expect "theteststring" theteststring -pgs 'q(.)' '\1z' 

# vop

re2expect "theteststring" theteststring -vop q 
re2expect "" theteststring -vop t 
re2expect "" theteststring -vop '(t.)' 
re2expect "theteststring" theteststring -vop '(q.)' 
re2expect "theteststring" theteststring -vops 't(.)' '\1z' 
re2expect "theteststring" theteststring -vops 'q(.)' '\1z' 

# vog

re2expect "theteststring" theteststring -vog q 
re2expect "" theteststring -vog t 
re2expect "" theteststring -vog '(t.)' 
re2expect "theteststring" theteststring -vog '(q.)' 
re2expect "" theteststring -vogs 't(.)' '\1z' 
re2expect "theteststring" theteststring -vogs 'q(.)' '\1z' 


# vpg

re2expect "theteststring" theteststring -vpg q 
re2expect "" theteststring -vpg t 
re2expect "" theteststring -vpg '(t.)' 
re2expect "theteststring" theteststring -vpg '(q.)' 
re2expect "theteststring" theteststring -vpgs 't(.)' '\1z' 
re2expect "theteststring" theteststring -vpgs 'q(.)' '\1z' 

# opg

re2expect "" theteststring -opg q 
re2expect "tttt" theteststring -opg t 
re2expect "thtetstr" theteststring -opg '(t.)' 
re2expect "" theteststring -opg '(q.)' 
re2expect "hzezszrz" theteststring -opgs 't(.)' '\1z' 
re2expect "theteststring" theteststring -opgs 'q(.)' '\1z' 

# vopg

re2expect "theteststring" theteststring -vopg q 
re2expect "" theteststring -vopg t 
re2expect "" theteststring -vopg '(t.)' 
re2expect "theteststring" theteststring -vopg '(q.)' 
re2expect "theteststring" theteststring -vopgs 't(.)' '\1z' 
re2expect "theteststring" theteststring -vopgs 'q(.)' '\1z' 

diff -q <(grep q tests/test.sh)  <($re2g q tests/test.sh) || fail=$(expr 1 + $fail);

diff -q <(grep -v q tests/test.sh)  <($re2g -v q tests/test.sh) || fail=$(expr 1 + $fail);

diff -q <(grep -h q tests/test.sh)  <($re2g -h q tests/test.sh) || fail=$(expr 1 + $fail);

diff -q <(grep -H q tests/test.sh)  <($re2g -H q tests/test.sh) || fail=$(expr 1 + $fail);


diff -q <(grep red /usr/share/dict/propernames /usr/share/dict/words)  <($re2g red /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(grep -h red /usr/share/dict/propernames /usr/share/dict/words)  <($re2g -h red /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(grep -H red /usr/share/dict/propernames /usr/share/dict/words)  <($re2g -H red /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(echo "food" | grep -H foo)  <(echo "food" | $re2g -H foo) || fail=$(expr 1 + $fail);

if [ $fail -gt 0 ]; then
  echo $fail Errors
  exit 1;
fi
