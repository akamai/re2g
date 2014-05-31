#!/bin/bash

re2g=$1;
grep=$2;

if [ -z "$grep" ]; then
  grep=grep;
fi

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

function test_same () {
  t="$1";
  shift;
  if diff -q $1 $2; then
    echo "SUCCESS: $grep ($t)";
  else
    echo "FAULURE: $grep ($t)";
    fail=`expr $fail + 1`;
  fi
}

if diff <($re2g -?) <(sed 's/%1\$s/'`basename $re2g`'/g' src/usage); then
  echo SUCCESS "-? => USAGE";
else
  echo FAILURE "-h => help has diverged"
  fail=1;
fi 

V=`$re2g --version`
if echo $V | $grep -q "^`basename $re2g` v[0-9]\{1,\}\.[0-9]\{1,\}\.[0-9]\{1,\}$"; then
  echo SUCCESS "--version => $V";
else
  echo FAILURE "--version => $V"
  fail=1;
fi 



# -o and -g can only be combined if build/gext.test prints "01"
HAVE_GLOBALEXTRACT=`build/gext.test`

if [ "01" == "$HAVE_GLOBALEXTRACT" ]; then
  echo "WARNING: built the good version (GlobalExtract), so unable to test error from using unsupported flags. This is a good thing."
else
  if [ "`$re2g -og foo <<<foo 2>&1`" = "Compiled against libre2 which lacks the GlobalExtract patch, don't use -g with -o" ]; then
    echo "SUCCESS: warned that this is the version which can't combine -o and -g flags";
  else
    echo "FAILURE: didn't warn about problems combing -o and -g, but this libre2 doesn't support GlobalExtract"
    fail=`expr $fail + 1`;
  fi
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

re2expect 1 "" foo tests #directory should fail

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
if [ "01" = "$HAVE_GLOBALEXTRACT" ]; then
  re2expect 1 "" theteststring -og q 
  re2expect 0 "tttt" theteststring -og t 
  re2expect 0 "thtetstr" theteststring -og '(t.)' 
  re2expect 1 "" theteststring -og '(q.)' 
  re2expect 0 "hzezszrz" theteststring -ogs '\1z' 't(.)' 
  re2expect 1 "" theteststring -ogs '\1z' 'q(.)' 
else
  echo "WARNING: built without support for GlobalExtract, skipping tests which combine -o and -g";
fi;

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

if [ "01" = "$HAVE_GLOBALEXTRACT" ]; then
  re2expect 1 "" theteststring -opg q 
  re2expect 0 "tttt" theteststring -opg t 
  re2expect 0 "thtetstr" theteststring -opg '(t.)' 
  re2expect 1 "" theteststring -opg '(q.)' 
  re2expect 0 "hzezszrz" theteststring -opgs '\1z' 't(.)' 
  re2expect 1 "theteststring" theteststring -opgs '\1z' 'q(.)'
else
  echo "WARNING: built without support for GlobalExtract, skipping tests which combine -o and -g";
fi;

# vopg

if [ "01" = "$HAVE_GLOBALEXTRACT" ]; then
  re2expect 0 "theteststring" theteststring -vopg q 
  re2expect 1 "" theteststring -vopg t 
  re2expect 1 "" theteststring -vopg '(t.)' 
  re2expect 0 "theteststring" theteststring -vopg '(q.)' 
  re2expect 1 "theteststring" theteststring -vopgs '\1z' 't(.)' 
  re2expect 0 "theteststring" theteststring -vopgs '\1z' 'q(.)' 
else
  echo "WARNING: built without support for GlobalExtract, skipping tests which combine -o and -g";
fi;


#match-any vs match-all
  re2expect 0 "theteststring" theteststring -e the -e test
  re2expect 0 "theteststring" theteststring -U -e the -e test
  re2expect 0 "theteststring" theteststring -V -e the -e test

  re2expect 0 "theteststring" theteststring -e the -e test -e fred
  re2expect 0 "theteststring" theteststring -U -e the -e test -e fred
  re2expect 1 "" theteststring -V -e the -e test -e fred


  re2expect 1 "" theteststring -v -e the -e test
  re2expect 1 "" theteststring -v -U -e the -e test
  re2expect 1 "" theteststring -v -V -e the -e test

  re2expect 1 "" theteststring -v -e the -e test -e fred
  re2expect 0 "theteststring" theteststring -v -U -e the -e test -e fred
  re2expect 1 "" theteststring -v -V -e the -e test -e fred


test_same 'char search' <($grep q tests/tests.sh)  <($re2g q tests/tests.sh)

test_same 'neg char-search' <($grep -v q tests/tests.sh)  <($re2g -v q tests/tests.sh)

test_same 'noflist' <($grep -h q tests/tests.sh)  <($re2g -h q tests/tests.sh)

test_same 'flist' <($grep -H q tests/tests.sh)  <($re2g -H q tests/tests.sh)


test_same 'multi-file' <($grep red tests/word.list tests/sigword.list)  <($re2g red tests/word.list tests/sigword.list)

test_same 'multi-file noflist' <($grep -h red tests/word.list tests/sigword.list)  <($re2g -h red tests/word.list tests/sigword.list)

test_same 'multi-file flist' <($grep -H red tests/word.list tests/sigword.list)  <($re2g -H red tests/word.list tests/sigword.list)

test_same 'multi-file count' <($grep -c red tests/word.list tests/sigword.list)  <($re2g -c red tests/word.list tests/sigword.list)

test_same 'multi-file neg-count' <($grep -vc red tests/word.list tests/sigword.list)  <($re2g -vc red tests/word.list tests/sigword.list)

test_same 'whole-line' <($grep -x Fred tests/word.list tests/sigword.list)  <($re2g -x Fred tests/word.list tests/sigword.list)

test_same 'case-insensitive' <($grep -i Fred tests/word.list tests/sigword.list)  <($re2g -i Fred tests/word.list tests/sigword.list)

test_same 'stdin default' <(echo "food" | grep -H foo)  <(echo "food" | $re2g -H foo)

test_same 'fixed' <(echo "fred" | grep -F f.)  <(echo "fred" | $re2g -F f.)

test_same 'fixed with dot' <(echo "f.red" | grep -F f.)  <(echo "f.red" | $re2g -F f.)

test_same 'list files' <($grep -l re2e src/*.cc tests/*.sh) <($re2g -l re2e src/*.cc tests/*.sh)

test_same 'neg list files' <($grep -vl re2e src/*.cc tests/*.sh) <($re2g -vl re2e src/*.cc tests/*.sh)


test_same 'list neg files' <($grep -L re2e src/*.cc tests/*.sh) <($re2g -L re2e src/*.cc tests/*.sh)

test_same 'neg list neg files' <($grep -vL re2e src/*.cc tests/*.sh) <($re2g -vL re2e src/*.cc tests/*.sh)

#uniq is used in the context test to remuve duplicated lines containing just '--'
#because my local OS X grep and Linux grep differ. re2g operates more like the linux grep

test_same 'context' <($grep --context=2 '[Aa]la' tests/word.list| uniq)  <($re2g --context=2 '[Aa]la' tests/word.list)

test_same '-C 9' <($grep -C 9 '[Aa]la' tests/word.list| uniq)  <($re2g -C 9 '[Aa]la' tests/word.list)

test_same '-B 7' <($grep -B 7 '[Aa]la' tests/word.list| uniq)  <($re2g -B 7 '[Aa]la' tests/word.list)

test_same '-A 12' <($grep -A 12 '[Aa]la' tests/word.list| uniq)  <($re2g -A 12 '[Aa]la' tests/word.list)


test_same '-HC 9' <($grep -HC 9 '[Aa]la' tests/word.list| uniq)  <($re2g -HC 9 '[Aa]la' tests/word.list)

test_same '-HB 7' <($grep -HB 7 '[Aa]la' tests/word.list| uniq)  <($re2g -HB 7 '[Aa]la' tests/word.list)

test_same '-HA 12' <($grep -HA 12 '[Aa]la' tests/word.list| uniq)  <($re2g -HA 12 '[Aa]la' tests/word.list)


test_same 'max 2' <($grep -m 2 '[Aa]la' tests/word.list| uniq)  <($re2g -m 2 '[Aa]la' tests/word.list)

test_same 'max 5' <($grep -m 5 '[Aa]la' tests/word.list| uniq)  <($re2g -m 5 '[Aa]la' tests/word.list)

#wanted to test against bigger dictionaries, but local grep is borked wrt context
#and is repeating lines!
#
#15063-ateuchi
#15064-ateuchus
#15065:Atfalati
#15066-Athabasca
#15067-Athabascan
#--
#15066-Athabasca
#15067-Athabascan
#15068:athalamous
#15069-athalline

test_same 'multi-file-context' <($grep --context=2 '[Aa]la' tests/word.list tests/lorem| uniq)  <($re2g --context=2 '[Aa]la' tests/word.list tests/lorem)

test_same 'changing group-separator' <($grep --context=2 '[Aa]la' tests/word.list| uniq |sed 's/^--$/QQ/')  <($re2g --context=2 -t QQ '[Aa]la' tests/word.list)

test_same 'no group-separator' <($grep --context=2 '[Aa]la' tests/word.list| grep -v '^--$')  <($re2g --context=2 -T '[Aa]la' tests/word.list)

test_same 'util rev' <(rev tests/lorem | $re2g rolod)  <($re2g -X rev \; rolod tests/lorem)

test_same 'util rev sharing stdin' <(rev tests/lorem | $re2g rolod)  <($re2g -X rev \; rolod < tests/lorem)

if [ -x `which gzip` ]; then
  unset gzflag;
  gzip -c < tests/lorem > build/lorem.gz
  if $grep -q -z dolor build/lorem.gz 2>/dev/null; then
    gzflag=z
  else
    if $grep -q -Z dolor build/lorem.gz 2>/dev/null; then
      echo 'WARNING: grep on this platform does not support -z to gunzip input, does support -Z';
      gzflag=Z;
    else
      echo 'WARNING: grep on this platform does not support -z or -Z to gunzip input, skipping gzip tests';
    fi
  fi
  if [ -s "$gzflag" ]; then
    test_same "unzip with $gzflag vs -X {}" <($grep -$gzflag dolor build/lorem.gz)  <($re2g -X gunzip -c '{}' \; dolor build/lorem.gz)
    test_same "unzip with $gzflag" <($grep -$gzflag dolor build/lorem.gz)  <($re2g -z dolor build/lorem.gz)
    test_same "unzip with $gzflag vs -X ;" <($grep -$gzflag dolor build/lorem.gz)  <($re2g -X gunzip \; dolor build/lorem.gz)

    if [ ! -f build/lorem.gz ]; then
      echo 'FAILED: call to gunzip deleted test file'
      fail=$(expr 1 + $fail);
    fi
  fi
else
  echo 'WARNING: Unable to find gzip, skipping -z tests'
fi

if [ -x "`which compress`" ]; then
  compress -c < tests/lorem > build/lorem_c.Z
  if diff -q <($grep -Z dolor build/lorem_c.Z) <(uncompress -c build/lorem_c.Z | grep dolor) >/dev/null; then
    test_same 'unzip with -Z vs -X uncompress {}' <($grep -Z dolor build/lorem_c.Z)  <($re2g -X uncompress -c '{}' \; dolor build/lorem_c.Z)
    test_same 'unzip with -Z vs -x zcat' <($grep -Z dolor build/lorem_c.Z)  <($re2g -X zcat '{}' \; dolor build/lorem_c.Z)
    test_same 'unzip with -Z for both' <($grep -Z dolor build/lorem_c.Z)  <($re2g -Z dolor build/lorem_c.Z)
    test_same 'unzip with -Z vs -X uncompress ;' <($grep -Z dolor build/lorem_c.Z)  <($re2g -X uncompress \; dolor build/lorem_c.Z)

    if [ ! -f build/lorem_c.Z ]; then
      echo FAILED: call to uncompress deleted test file
      fail=$(expr 1 + $fail);
    fi
  else
    echo 'WARNING: grep on this platform does not support -Z to uncompress .Z files, skipping compress tests';
  fi
else
  echo 'WARNING: Unable to find compress, skipping -Z tests'
fi



if [ -x `which bzip2` ]; then
  bzip2 -c < tests/lorem > build/lorem.bz2
  if $grep -qJ dolor build/lorem.bz2 2>/dev/null >/dev/null; then
    test_same '-J vs -X bunzip2 -c {}' <($grep -J dolor build/lorem.bz2)  <($re2g -X bunzip2 -c '{}' \; dolor build/lorem.bz2)
    test_same '-J vs -J' <($grep -J dolor build/lorem.bz2)  <($re2g -J dolor build/lorem.bz2)
    test_same '-J vs -X bunzip2 ;' <($grep -J dolor build/lorem.bz2)  <($re2g -X bunzip2 \; dolor build/lorem.bz2)
    
    if [ ! -f build/lorem.bz2 ]; then
      echo FAILED: call to bunzip2 deleted test file
      fail=$(expr 1 + $fail);
    fi
  else
  echo 'WARNING: grep on this platform does not support -J to unbzip2 input, skipping bzip2 tests'
  fi
else
  echo 'WARNING: Unable to find bzip, skipping -J tests'
fi


test_same 'exit code test without -q' <($grep dolor tests/lorem; echo $?)  <($re2g dolor tests/lorem; echo $?)


test_same '1 exit code test with -q' <($grep -q dolor tests/lorem; echo $?)  <($re2g -q dolor tests/lorem; echo $?)

test_same '0 exit code test with -q' <($grep -q dolr tests/lorem; echo $?)  <($re2g -q dolr tests/lorem; echo $?)


test_same '-E and exit code' <($grep -E 'ipsum|ex' tests/lorem; echo $?)  <($re2g -E 'ipsum|ex' tests/lorem; echo $?)
#todo add a test where \b is tested on platforms where grep isn't crazy


if $re2g -l  fred tests/bad*|xargs -I% test -f "%";then
  echo 'FAILURE: unable to detect newline in filename';
  fail=$(expr 1 + $fail);
else
  echo 'SUCCESS: newline in filename breaks xargs'
fi


if $re2g -l0  fred tests/bad*|xargs -I% -0 test -f "%" ;then
  echo 'SUCCESS: able to defeat newline in filename';
else
  echo 'FAILURE: unable to defeat newline in filename';
  fail=$(expr 1 + $fail);
fi

#not yet testing for line-buffering


test_same 'pattern-file' <($grep -f tests/pats tests/lorem)  <($re2g -f tests/pats tests/lorem)

test_same 'neg pattern-file' <($grep -vf tests/pats tests/lorem)  <($re2g -vf tests/pats tests/lorem)

test_same 'neg pattern-file' <($grep -vf tests/pats tests/lorem)  <($re2g -vVf tests/pats tests/lorem)


#grep -o with multiple patterns is a mess.
#let's just store the previous result and look for changes
#test_same '' <($grep -of tests/pats tests/lorem)  <($re2g -of tests/pats tests/lorem)

if diff -q tests/nof_pats_lorem <($re2g -nof tests/pats tests/lorem); then
  echo 'SUCCESS: patfile -nof'
else 
  echo 'FAILURE: patfile -nof'
  fail=$(expr 1 + $fail);
fi

if diff -q tests/of_pats_lorem <($re2g -of tests/pats tests/lorem); then
  echo 'SUCCESS: patfile -of'
else 
  echo 'FAILURE: patfile -of'
  fail=$(expr 1 + $fail);
fi


if [ $fail -gt 0 ]; then
  echo $fail Errors
  exit 1;
else
  echo "No Errors"
  exit 0;
fi

