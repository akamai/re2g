# RE2G

This is a grep-alike which uses RE2.

For information about the goals of this project, see: src/re2g.cc


# Prerequisite: RE2

You need to have RE2 built and the RE2 headers available. If you use a
version of RE2 with my GlobalExtract patch, you'll get more
functionality out of re2g, and the test suite will pass. The patch can
be found in `global_extract.patch`

The build and test process further expects to find bash at `/bin/bash`
and the following in the `$PATH: od, sed, grep, diff, zcat, rev,
uncompress, bzip2`. If you are missing any of these you won't be able
to run the testcases and building will become more manual.


# Compiling

If RE2 is installed normally, just run `make`.

If RE2 is built, but not installed, try a variation on this:

    make RE2_LIB=-L/path/to/re2/lib  RE2_INC=-I/path/to/re2/include/dir


For example, if you built RE2 in `../re2`, this would work:

    make RE2_LIB=-L../re2/obj RE2_INC=-I../re2


If RE2 is installed in an unusual location, you do something like:

    make RE2_LIB=-L/opt/re2/lib/ RE2_INC=-I/opt/re2/include

Alternatively, copy `Makefile.local.example` to `Makefile.local` and
edit the values for `RE2_LIB` and `RE2_INC` there. Then you can just
run Make without setting vars on the command line.

Run `make clean` when changing these variables or `Makefile.local`.


# Testing

`make test` will invoke a shell script for testing your built version
of re2g. It's a bit noisy. I tend to run `make test | grep -v SUCCESS`
to find out what's not working.

Since the tests compare output to grep, you may want to use a specific
grep. The tests pass against gnu grep 2.14. You can use a specific
grep by setting the `USE_GREP` make variable:

    make test USE_GREP=../grep-2.14/src/grep

There is an example in `Makefile.local.example` for this as well.

Run `make clean` when changing these variables or altering
`Makefile.local`.


# Installing

You can just copy `build/re2g` to wherever you want it, or run `make install`

To override the install directory override the `PREFIX` or `BINDIR`
make variables on the command line or in your `Makefile.local`.

`make install` will install into `$(BINDIR)` which defaults to
`$(PREFIX)/bin`

`$(PREFIX)` defaults to `/usr/local`

There are examples in `Makefile.local.example`.


# Notes

tests/word.list and tests/sigwords.list are from the public
domain YAWL from http://www.cs.duke.edu/csed/data/yawl-0.3.2/
