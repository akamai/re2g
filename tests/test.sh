#!/bin/bash

re2g=$1;

fail=0;

function re2expect () {
  r="$1";
  e="$2";
  t="$3";
  shift 3;

  o=$(printf -- "$t"|$re2g "$@");
  s=$?;


  if [ $s = $r ]; then 
    if [ "$e" = "$o" ]; then
      echo SUCCESS "$re2g $@ <<<\"$t\"" '=>' "'$e'";
    else
      echo FAILURE "$re2g $@ <<<\"$t\"" '=>' "'$o'" NOT "'$e'";
      fail=`expr $fail + 1`;
    fi
  else
    echo FAILURE "$re2g $@ <<<\"$t\"" ' ERR ' $s; 
    fail=`expr $fail + 1`;
  fi
}


if [ $($re2g -?|md5) = 51fd57e693c91ffd6b82486dd3f48e60 ]; then
  echo SUCCESS "-? => USAGE";
else
  echo FAILURE "-h => help has diverged"
  fail=1;
fi 

re2expect 0 fred fred f 

re2expect 1 "" fred b 

re2expect 0 re fred -o r.  

re2expect 0 re fred -oe r.  

re2expect 0 re fred -oe q -e r.  

re2expect 1 "" fred -v r.  

re2expect 1 "" fred -vo r.  
 

re2expect 0 fxed fred r -s x 

re2expect 1 fred fred q -ps z  

re2expect 0 fred fred q -vs z  

re2expect 1 "" fred q -s z 

re2expect 0 fxdrob fredrob r. -s x 

re2expect 0 fxdxb fredrob r. -gs x  

re2expect 0 fexdoxb fredrob 'r(.)' -gs '\1x'  


# NONE

re2expect 1 "" theteststring q
re2expect 0 "theteststring" theteststring t
re2expect 0 "theteststring" theteststring '(t.)'
re2expect 1 "" theteststring '(q.)'
re2expect 0 "hzeteststring" theteststring -s '\1z' 't(.)' 
re2expect 1 "" theteststring -s '\1z' 'q(.)' 

# v

re2expect 0 "theteststring" theteststring -v  q 
re2expect 1 "" theteststring -v t 
re2expect 1 "" theteststring -v '(t.)' 
re2expect 0 "theteststring" theteststring -v '(q.)' 
re2expect 1 "" theteststring -vs '\1z' 't(.)'
re2expect 0 "theteststring" theteststring -vs '\1z' 'q(.)'

# o

re2expect 1 "" theteststring -o q 
re2expect 0 "t" theteststring -o t 
re2expect 0 "th" theteststring -o '(t.)' 
re2expect 1 "" theteststring -o '(q.)' 
re2expect 0 "hz" theteststring -os  '\1z' 't(.)'
re2expect 1 "" theteststring -os  '\1z' 'q(.)'

# p

re2expect 1 "" theteststring -p q 
re2expect 0 "theteststring" theteststring -p t 
re2expect 0 "theteststring" theteststring -p '(t.)' 
re2expect 1 "" theteststring -p '(q.)' 
re2expect 0 "hzeteststring" theteststring -ps '\1z' 't(.)'
re2expect 1 "theteststring" theteststring -ps '\1z' 'q(.)'

# g

re2expect 1 "" theteststring -g q 
re2expect 0 "theteststring" theteststring -g t 
re2expect 0 "theteststring" theteststring -g '(t.)' 
re2expect 1 "" theteststring -g '(q.)' 
re2expect 0 "hzeezsszrzing" theteststring -gs '\1z' 't(.)'
re2expect 1 "" theteststring -gs '\1z' 'q(.)'

# vo

re2expect 0 "theteststring" theteststring -vo q 
re2expect 1 "" theteststring -vo t 
re2expect 1 "" theteststring -vo '(t.)' 
re2expect 0 "theteststring" theteststring -vo '(q.)' 
re2expect 1 "" theteststring -vos '\1z' 't(.)'
re2expect 0 "theteststring" theteststring -vos '\1z' 'q(.)'

# vp

re2expect 0 "theteststring" theteststring -vp q 
re2expect 1 "" theteststring -vp t 
re2expect 1 "" theteststring -vp '(t.)' 
re2expect 0 "theteststring" theteststring -vp '(q.)' 
re2expect 1 "theteststring" theteststring -vps '\1z' 't(.)'
re2expect 0 "theteststring" theteststring -vps '\1z' 'q(.)'

# vg

re2expect 0 "theteststring" theteststring -vg  q 
re2expect 1 "" theteststring -vg t 
re2expect 1 "" theteststring -vg '(t.)' 
re2expect 0 "theteststring" theteststring -vg '(q.)' 
re2expect 1 "" theteststring -vgs '\1z' 't(.)'
re2expect 0 "theteststring" theteststring -vgs '\1z' 'q(.)'

# op

re2expect 1 "" theteststring -op q 
re2expect 0 "t" theteststring -op t 
re2expect 0 "th" theteststring -op '(t.)' 
re2expect 1 "" theteststring -op '(q.)' 
re2expect 0 "hz" theteststring -ops '\1z' 't(.)' 
re2expect 1 "theteststring" theteststring -ops '\1z' 'q(.)' 

# og

re2expect 1 "" theteststring -og q 
re2expect 0 "tttt" theteststring -og t 
re2expect 0 "thtetstr" theteststring -og '(t.)' 
re2expect 1 "" theteststring -og '(q.)' 
re2expect 0 "hzezszrz" theteststring -ogs '\1z' 't(.)' 
re2expect 1 "" theteststring -ogs '\1z' 'q(.)' 

# pg

re2expect 1 "" theteststring -pg q 
re2expect 0 "theteststring" theteststring -pg t 
re2expect 0 "theteststring" theteststring -pg '(t.)' 
re2expect 1 "" theteststring -pg '(q.)' 
re2expect 0 "hzeezsszrzing" theteststring -pgs '\1z' 't(.)' 
re2expect 1 "theteststring" theteststring -pgs '\1z' 'q(.)' 

# vop

re2expect 0 "theteststring" theteststring -vop q 
re2expect 1 "" theteststring -vop t 
re2expect 1 "" theteststring -vop '(t.)' 
re2expect 0 "theteststring" theteststring -vop '(q.)' 
re2expect 1 "theteststring" theteststring -vops '\1z' 't(.)' 
re2expect 0 "theteststring" theteststring -vops '\1z' 'q(.)' 

# vog

re2expect 0 "theteststring" theteststring -vog q 
re2expect 1 "" theteststring -vog t 
re2expect 1 "" theteststring -vog '(t.)' 
re2expect 0 "theteststring" theteststring -vog '(q.)' 
re2expect 1 "" theteststring -vogs '\1z' 't(.)' 
re2expect 0 "theteststring" theteststring -vogs '\1z' 'q(.)' 


# vpg

re2expect 0 "theteststring" theteststring -vpg q 
re2expect 1 "" theteststring -vpg t 
re2expect 1 "" theteststring -vpg '(t.)' 
re2expect 0 "theteststring" theteststring -vpg '(q.)' 
re2expect 1 "theteststring" theteststring -vpgs '\1z' 't(.)' 
re2expect 0 "theteststring" theteststring -vpgs '\1z' 'q(.)' 

# opg

re2expect 1 "" theteststring -opg q 
re2expect 0 "tttt" theteststring -opg t 
re2expect 0 "thtetstr" theteststring -opg '(t.)' 
re2expect 1 "" theteststring -opg '(q.)' 
re2expect 0 "hzezszrz" theteststring -opgs '\1z' 't(.)' 
re2expect 1 "theteststring" theteststring -opgs '\1z' 'q(.)' 

# vopg

re2expect 0 "theteststring" theteststring -vopg q 
re2expect 1 "" theteststring -vopg t 
re2expect 1 "" theteststring -vopg '(t.)' 
re2expect 0 "theteststring" theteststring -vopg '(q.)' 
re2expect 1 "theteststring" theteststring -vopgs '\1z' 't(.)' 
re2expect 0 "theteststring" theteststring -vopgs '\1z' 'q(.)' 

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

if [ -x `which gzip` ]; then
  gzip -c < tests/lorem > tests/lorem.gz
  if grep -qz dolor tests/lorem.gz 2>/dev/null; then
    gzflag=z
  else
    echo 'grep on this platform is broken, does not support -z, trying with -Z';
    grep -qZ dolor tests/lorem.gz && gzflag=Z
  fi
  if [ -s $gzflag ]; then
    diff -q <(grep -$gzflag dolor tests/lorem.gz)  <($re2g -X gunzip -c '{}' \; dolor tests/lorem.gz) || fail=$(expr 1 + $fail);
    diff -q <(grep -$gzflag dolor tests/lorem.gz)  <($re2g -z dolor tests/lorem.gz) || fail=$(expr 1 + $fail);
    diff -q <(grep -$gzflag dolor tests/lorem.gz)  <($re2g -X gunzip \; dolor tests/lorem.gz) || fail=$(expr 1 + $fail);

    if [ ! -f tests/lorem.gz ]; then
      echo FAILED: call to gunzip deleted test file
      fail=$(expr 1 + $fail);
    fi
  fi
else
  echo 'Unable to find gzip, skipping -z tests'
fi

if [ -x `which zcat` ]; then
  compress -c < tests/lorem > tests/lorem_c.Z
  if diff -q <(grep -Z dolor tests/lorem_c.Z) <(uncompress -c tests/lorem_c.Z | grep dolor) >/dev/null; then
    diff -q <(grep -Z dolor tests/lorem_c.Z)  <($re2g -X uncompress -c '{}' \; dolor tests/lorem_c.Z) || fail=$(expr 1 + $fail);
    diff -q <(grep -Z dolor tests/lorem_c.Z)  <($re2g -X zcat '{}' \; dolor tests/lorem_c.Z) || fail=$(expr 1 + $fail);
    diff -q <(grep -Z dolor tests/lorem_c.Z)  <($re2g -Z dolor tests/lorem_c.Z) || fail=$(expr 1 + $fail);
    diff -q <(grep -Z dolor tests/lorem_c.Z)  <($re2g -X uncompress \; dolor tests/lorem_c.Z) || fail=$(expr 1 + $fail);

    if [ ! -f tests/lorem_c.Z ]; then
      echo FAILED: call to uncompress deleted test file
      fail=$(expr 1 + $fail);
    fi
  else
    echo 'grep on this platform is broken, -Z can'\''t uncompress .Z files' 
  fi
else
  echo 'Unable to find zcat, skipping -Z tests'
fi



if [ -x `which bzip2` ]; then
  bzip2 -c < tests/lorem > tests/lorem.bz2
  diff -q <(grep -J dolor tests/lorem.bz2)  <($re2g -X bunzip2 -c '{}' \; dolor tests/lorem.bz2) || fail=$(expr 1 + $fail);
  diff -q <(grep -J dolor tests/lorem.bz2)  <($re2g -J dolor tests/lorem.bz2) || fail=$(expr 1 + $fail);
  diff -q <(grep -J dolor tests/lorem.bz2)  <($re2g -X bunzip2 \; dolor tests/lorem.bz2) || fail=$(expr 1 + $fail);

  if [ ! -f tests/lorem.bz2 ]; then
    echo FAILED: call to bunzip2 deleted test file
    fail=$(expr 1 + $fail);
  fi
else
  echo 'Unable to find bzip, skipping -J tests'
fi


diff -q <(grep dolor tests/lorem; echo $?)  <($re2g dolor tests/lorem; echo $?) || fail=$(expr 1 + $fail);


diff -q <(grep -q dolor tests/lorem; echo $?)  <($re2g -q dolor tests/lorem; echo $?) || fail=$(expr 1 + $fail);

diff -q <(grep -q dolr tests/lorem; echo $?)  <($re2g -q dolr tests/lorem; echo $?) || fail=$(expr 1 + $fail);


diff -q <(grep -E 'ipsum|ex' tests/lorem; echo $?)  <($re2g -E 'ipsum|ex' tests/lorem; echo $?) || fail=$(expr 1 + $fail);
#todo add a test where \b is tested on platforms where grep isn't crazy


if $re2g -l  fred tests/bad*|xargs  stat -qf '' ;then
  echo 'FAILURE: unable to detect newline in filename';
  fail=$(expr 1 + $fail);
else
  echo 'SUCCESS: newline in filename breaks xargs'
fi


if $re2g -l0  fred tests/bad*|xargs -0 stat -qf '' ;then
  echo 'SUCCESS: able to defeat newline in filename';
else
  echo 'FAILURE: unable to defeat newline in filename';
  fail=$(expr 1 + $fail);
fi

#not yet testing for line-buffering


diff -q <(grep -f tests/pats tests/lorem)  <($re2g -f tests/pats tests/lorem) || fail=$(expr 1 + $fail);

#grep -o with multiple patterns is a mess.
#let's just store the previous result and look for changes
#diff -q <(grep -of tests/pats tests/lorem)  <($re2g -of tests/pats tests/lorem) || fail=$(expr 1 + $fail);

diff -q tests/nof_pats_lorem <($re2g -nof tests/pats tests/lorem) || fail=$(expr 1 + $fail);

diff -q tests/of_pats_lorem <($re2g -of tests/pats tests/lorem) || fail=$(expr 1 + $fail);


if [ $fail -gt 0 ]; then
  echo $fail Errors
  exit 1;
fi
