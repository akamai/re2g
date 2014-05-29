src=src
build=build
tests=tests

-include Makefile.local

CXX?=g++
CXXFLAGS?=-I$(src) -I$(build) -L$(build) -Wall $(RE2_INC) $(RE2_LIB)
LDFLAGS?=-lre2 -pthread

xargp=$(shell echo J|xargs -J% echo % 2>/dev/null|| echo I)

$(build)/re2g: $(src)/re2g.cc $(build)/re2g_usage.h $(build)/gext.test
	$(CXX) $(CXXFLAGS) -D HAVE_GLOBAL_EXTRACT=$(shell $(build)/gext.test) $< -o $@ $(LDFLAGS)

$(build)/gext.test: $(src)/gext.cc $(src)/gextbad.sh
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) || cp $(src)/gextbad.sh $@

$(build)/re2g_usage.h: $(src)/usage
	od -b -A n $(src)/usage|tr -d "\n"|xargs -$(xargp)% echo '#define RE2G_USAGE_STR {' % '0}'|sed 's/\([0-9]\{1,\}\) /0\1, /g' > $@

test: $(build)/re2g $(tests)/tests.sh
	$(tests)/tests.sh $(build)/re2g $(USE_GREP)

clean:
	$(RM) ./$(build)/*
