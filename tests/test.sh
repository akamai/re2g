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


if [ $($re2g -h|md5) = 2207057546ffc967a07db798bbf1ae34 ]; then
  echo SUCCESS "-h => USAGE";
else
  echo FAILURE "-h => help has diverged"
  fail=1;
fi 

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


# NONE

re2expect "" theteststring q
re2expect "theteststring" theteststring t
re2expect "theteststring" theteststring '(t.)'
re2expect "" theteststring '(q.)'
re2expect "hzeteststring" theteststring 't(.)' '\1z'
re2expect "" theteststring 'q(.)' '\1z'

# v

re2expect "theteststring" -v  theteststring q
re2expect "" -v theteststring t
re2expect "" -v theteststring '(t.)'
re2expect "theteststring" -v theteststring '(q.)'
re2expect "" -v theteststring 't(.)' '\1z'
re2expect "theteststring" -v theteststring 'q(.)' '\1z'

# o

re2expect "" -o theteststring q
re2expect "t" -o theteststring t
re2expect "th" -o theteststring '(t.)'
re2expect "" -o theteststring '(q.)'
re2expect "hz" -o  theteststring 't(.)' '\1z'
re2expect "" -o theteststring 'q(.)' '\1z'

# p

re2expect "" -p theteststring q
re2expect "theteststring" -p theteststring t
re2expect "theteststring" -p theteststring '(t.)'
re2expect "" -p theteststring '(q.)'
re2expect "hzeteststring" -p theteststring 't(.)' '\1z'
re2expect "theteststring" -p theteststring 'q(.)' '\1z'

# g

re2expect "" -g theteststring q
re2expect "theteststring" -g theteststring t
re2expect "theteststring" -g theteststring '(t.)'
re2expect "" -g theteststring '(q.)'
re2expect "hzeezsszrzing" -g theteststring 't(.)' '\1z'
re2expect "" -g theteststring 'q(.)' '\1z'

# vo

re2expect "theteststring" -vo  theteststring q
re2expect "" -vo theteststring t
re2expect "" -vo theteststring '(t.)'
re2expect "theteststring" -vo theteststring '(q.)'
re2expect "" -vo theteststring 't(.)' '\1z'
re2expect "theteststring" -vo theteststring 'q(.)' '\1z'

# vp

re2expect "theteststring" -vp  theteststring q
re2expect "" -vp theteststring t
re2expect "" -vp theteststring '(t.)'
re2expect "theteststring" -vp theteststring '(q.)'
re2expect "theteststring" -vp theteststring 't(.)' '\1z'
re2expect "theteststring" -vp theteststring 'q(.)' '\1z'

# vg

re2expect "theteststring" -vg  theteststring q
re2expect "" -vg theteststring t
re2expect "" -vg theteststring '(t.)'
re2expect "theteststring" -vg theteststring '(q.)'
re2expect "" -vg theteststring 't(.)' '\1z'
re2expect "theteststring" -vg theteststring 'q(.)' '\1z'

# op

re2expect "" -op theteststring q
re2expect "t" -op theteststring t
re2expect "th" -op theteststring '(t.)'
re2expect "" -op theteststring '(q.)'
re2expect "hz" -op theteststring 't(.)' '\1z'
re2expect "theteststring" -op theteststring 'q(.)' '\1z'

# og

re2expect "" -og theteststring q
re2expect "t" -og theteststring t
re2expect "th" -og theteststring '(t.)'
re2expect "" -og theteststring '(q.)'
re2expect "hz" -og theteststring 't(.)' '\1z'
re2expect "" -og theteststring 'q(.)' '\1z'

# pg

re2expect "" -pg theteststring q
re2expect "theteststring" -pg theteststring t
re2expect "theteststring" -pg theteststring '(t.)'
re2expect "" -pg theteststring '(q.)'
re2expect "hzeezsszrzing" -pg theteststring 't(.)' '\1z'
re2expect "theteststring" -pg theteststring 'q(.)' '\1z'

# vop

re2expect "theteststring" -vop  theteststring q
re2expect "" -vop theteststring t
re2expect "" -vop theteststring '(t.)'
re2expect "theteststring" -vop theteststring '(q.)'
re2expect "theteststring" -vop theteststring 't(.)' '\1z'
re2expect "theteststring" -vop theteststring 'q(.)' '\1z'

# vog

re2expect "theteststring" -vog  theteststring q
re2expect "" -vog theteststring t
re2expect "" -vog theteststring '(t.)'
re2expect "theteststring" -vog theteststring '(q.)'
re2expect "" -vog theteststring 't(.)' '\1z'
re2expect "theteststring" -vog theteststring 'q(.)' '\1z'


# vpg

re2expect "theteststring" -vpg  theteststring q
re2expect "" -vpg theteststring t
re2expect "" -vpg theteststring '(t.)'
re2expect "theteststring" -vpg theteststring '(q.)'
re2expect "theteststring" -vpg theteststring 't(.)' '\1z'
re2expect "theteststring" -vpg theteststring 'q(.)' '\1z'

# opg

re2expect "" -opg theteststring q
re2expect "t" -opg theteststring t
re2expect "th" -opg theteststring '(t.)'
re2expect "" -opg theteststring '(q.)'
re2expect "hz" -opg theteststring 't(.)' '\1z'
re2expect "theteststring" -opg theteststring 'q(.)' '\1z'

# vopg

re2expect "theteststring" -vopg theteststring q
re2expect "" -vopg theteststring t
re2expect "" -vopg theteststring '(t.)'
re2expect "theteststring" -vopg theteststring '(q.)'
re2expect "theteststring" -vopg theteststring 't(.)' '\1z'
re2expect "theteststring" -vopg theteststring 'q(.)' '\1z'




if [ $fail -gt 0 ]; then
  echo $fail Errors
  exit 1;
fi
