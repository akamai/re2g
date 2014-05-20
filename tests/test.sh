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


if [ $($re2g -h|md5) = a9e68b8999f418cd2ce8ad7e0c1cf0e7 ]; then
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
 

re2expect fxed fred r -s x 

re2expect fred fred q -ps z  

re2expect fred fred q -vs z  

re2expect "" fred q -s z 

re2expect fxdrob fredrob r. -s x 

re2expect fxdxb fredrob r. -gs x  

re2expect fexdoxb fredrob 'r(.)' -gs '\1x'  


# NONE

re2expect "" theteststring q
re2expect "theteststring" theteststring t
re2expect "theteststring" theteststring '(t.)'
re2expect "" theteststring '(q.)'
re2expect "hzeteststring" theteststring -s '\1z' 't(.)' 
re2expect "" theteststring -s '\1z' 'q(.)' 

# v

re2expect "theteststring" theteststring -v  q 
re2expect "" theteststring -v t 
re2expect "" theteststring -v '(t.)' 
re2expect "theteststring" theteststring -v '(q.)' 
re2expect "" theteststring -vs '\1z' 't(.)'
re2expect "theteststring" theteststring -vs '\1z' 'q(.)'

# o

re2expect "" theteststring -o q 
re2expect "t" theteststring -o t 
re2expect "th" theteststring -o '(t.)' 
re2expect "" theteststring -o '(q.)' 
re2expect "hz" theteststring -os  '\1z' 't(.)'
re2expect "" theteststring -os  '\1z' 'q(.)'

# p

re2expect "" theteststring -p q 
re2expect "theteststring" theteststring -p t 
re2expect "theteststring" theteststring -p '(t.)' 
re2expect "" theteststring -p '(q.)' 
re2expect "hzeteststring" theteststring -ps '\1z' 't(.)'
re2expect "theteststring" theteststring -ps '\1z' 'q(.)'

# g

re2expect "" theteststring -g q 
re2expect "theteststring" theteststring -g t 
re2expect "theteststring" theteststring -g '(t.)' 
re2expect "" theteststring -g '(q.)' 
re2expect "hzeezsszrzing" theteststring -gs '\1z' 't(.)'
re2expect "" theteststring -gs '\1z' 'q(.)'

# vo

re2expect "theteststring" theteststring -vo q 
re2expect "" theteststring -vo t 
re2expect "" theteststring -vo '(t.)' 
re2expect "theteststring" theteststring -vo '(q.)' 
re2expect "" theteststring -vos '\1z' 't(.)'
re2expect "theteststring" theteststring -vos '\1z' 'q(.)'

# vp

re2expect "theteststring" theteststring -vp q 
re2expect "" theteststring -vp t 
re2expect "" theteststring -vp '(t.)' 
re2expect "theteststring" theteststring -vp '(q.)' 
re2expect "theteststring" theteststring -vps '\1z' 't(.)'
re2expect "theteststring" theteststring -vps '\1z' 'q(.)'

# vg

re2expect "theteststring" theteststring -vg  q 
re2expect "" theteststring -vg t 
re2expect "" theteststring -vg '(t.)' 
re2expect "theteststring" theteststring -vg '(q.)' 
re2expect "" theteststring -vgs '\1z' 't(.)'
re2expect "theteststring" theteststring -vgs '\1z' 'q(.)'

# op

re2expect "" theteststring -op q 
re2expect "t" theteststring -op t 
re2expect "th" theteststring -op '(t.)' 
re2expect "" theteststring -op '(q.)' 
re2expect "hz" theteststring -ops '\1z' 't(.)' 
re2expect "theteststring" theteststring -ops '\1z' 'q(.)' 

# og

re2expect "" theteststring -og q 
re2expect "tttt" theteststring -og t 
re2expect "thtetstr" theteststring -og '(t.)' 
re2expect "" theteststring -og '(q.)' 
re2expect "hzezszrz" theteststring -ogs '\1z' 't(.)' 
re2expect "" theteststring -ogs '\1z' 'q(.)' 

# pg

re2expect "" theteststring -pg q 
re2expect "theteststring" theteststring -pg t 
re2expect "theteststring" theteststring -pg '(t.)' 
re2expect "" theteststring -pg '(q.)' 
re2expect "hzeezsszrzing" theteststring -pgs '\1z' 't(.)' 
re2expect "theteststring" theteststring -pgs '\1z' 'q(.)' 

# vop

re2expect "theteststring" theteststring -vop q 
re2expect "" theteststring -vop t 
re2expect "" theteststring -vop '(t.)' 
re2expect "theteststring" theteststring -vop '(q.)' 
re2expect "theteststring" theteststring -vops '\1z' 't(.)' 
re2expect "theteststring" theteststring -vops '\1z' 'q(.)' 

# vog

re2expect "theteststring" theteststring -vog q 
re2expect "" theteststring -vog t 
re2expect "" theteststring -vog '(t.)' 
re2expect "theteststring" theteststring -vog '(q.)' 
re2expect "" theteststring -vogs '\1z' 't(.)' 
re2expect "theteststring" theteststring -vogs '\1z' 'q(.)' 


# vpg

re2expect "theteststring" theteststring -vpg q 
re2expect "" theteststring -vpg t 
re2expect "" theteststring -vpg '(t.)' 
re2expect "theteststring" theteststring -vpg '(q.)' 
re2expect "theteststring" theteststring -vpgs '\1z' 't(.)' 
re2expect "theteststring" theteststring -vpgs '\1z' 'q(.)' 

# opg

re2expect "" theteststring -opg q 
re2expect "tttt" theteststring -opg t 
re2expect "thtetstr" theteststring -opg '(t.)' 
re2expect "" theteststring -opg '(q.)' 
re2expect "hzezszrz" theteststring -opgs '\1z' 't(.)' 
re2expect "theteststring" theteststring -opgs '\1z' 'q(.)' 

# vopg

re2expect "theteststring" theteststring -vopg q 
re2expect "" theteststring -vopg t 
re2expect "" theteststring -vopg '(t.)' 
re2expect "theteststring" theteststring -vopg '(q.)' 
re2expect "theteststring" theteststring -vopgs '\1z' 't(.)' 
re2expect "theteststring" theteststring -vopgs '\1z' 'q(.)' 

diff -q <(grep q tests/test.sh)  <($re2g q tests/test.sh) || fail=$(expr 1 + $fail);

diff -q <(grep -v q tests/test.sh)  <($re2g -v q tests/test.sh) || fail=$(expr 1 + $fail);

diff -q <(grep -h q tests/test.sh)  <($re2g -h q tests/test.sh) || fail=$(expr 1 + $fail);

diff -q <(grep -H q tests/test.sh)  <($re2g -H q tests/test.sh) || fail=$(expr 1 + $fail);


diff -q <(grep red /usr/share/dict/propernames /usr/share/dict/words)  <($re2g red /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(grep -h red /usr/share/dict/propernames /usr/share/dict/words)  <($re2g -h red /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(grep -H red /usr/share/dict/propernames /usr/share/dict/words)  <($re2g -H red /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(grep -c red /usr/share/dict/propernames /usr/share/dict/words)  <($re2g -c red /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(grep -vc red /usr/share/dict/propernames /usr/share/dict/words)  <($re2g -vc red /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(grep -x Fred /usr/share/dict/propernames /usr/share/dict/words)  <($re2g -x Fred /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(grep -i Fred /usr/share/dict/propernames /usr/share/dict/words)  <($re2g -i Fred /usr/share/dict/propernames /usr/share/dict/words) || fail=$(expr 1 + $fail);

diff -q <(echo "food" | grep -H foo)  <(echo "food" | $re2g -H foo) || fail=$(expr 1 + $fail);

diff -q <(echo "fred" | grep -F f.)  <(echo "fred" | $re2g -F f.) || fail=$(expr 1 + $fail);

diff -q <(echo "f.red" | grep -F f.)  <(echo "f.red" | $re2g -F f.) || fail=$(expr 1 + $fail);

diff -q <(grep -l re2e src/*.cxx tests/*.sh) <($re2g -l re2e src/*.cxx tests/*.sh) || fail=$(expr 1 + $fail);

diff -q <(grep -vl re2e src/*.cxx tests/*.sh) <($re2g -vl re2e src/*.cxx tests/*.sh) || fail=$(expr 1 + $fail);


diff -q <(grep -L re2e src/*.cxx tests/*.sh) <($re2g -L re2e src/*.cxx tests/*.sh) || fail=$(expr 1 + $fail);

diff -q <(grep -vL re2e src/*.cxx tests/*.sh) <($re2g -vL re2e src/*.cxx tests/*.sh) || fail=$(expr 1 + $fail);


diff -q <(grep --context '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g --context '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);

diff -q <(grep -C 9 '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g -C 9 '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);

diff -q <(grep -B 7 '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g -B 7 '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);

diff -q <(grep -A 12 '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g -A 12 '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);


diff -q <(grep -HC 9 '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g -HC 9 '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);

diff -q <(grep -HB 7 '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g -HB 7 '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);

diff -q <(grep -HA 12 '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g -HA 12 '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);


diff -q <(grep -m 2 '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g -m 2 '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);

diff -q <(grep -m 5 '[Aa]la' /usr/share/dict/propernames|grep -v '^--$')  <($re2g -m 5 '[Aa]la' /usr/share/dict/propernames) || fail=$(expr 1 + $fail);

diff -q <(rev tests/lorem | $re2g rolod)  <($re2g -X rev \; rolod tests/lorem) || fail=$(expr 1 + $fail);

diff -q <(rev tests/lorem | $re2g rolod)  <($re2g -X rev \; rolod < tests/lorem) || fail=$(expr 1 + $fail);

if [ $fail -gt 0 ]; then
  echo $fail Errors
  exit 1;
fi
