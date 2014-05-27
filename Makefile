src=src
build=build
tests=tests

-include Makefile.local

CXX?=g++
CXXFLAGS?=-I$(src) -I$(build) -L$(build) -Wall -lre2 $(RE2_INC) $(RE2_LIB)

$(build)/re2g: $(src)/re2g.cc $(build)/re2g_usage.h $(build)/gext.test
	$(CXX) $(CXXFLAGS) -D HAVE_GLOBAL_EXTRACT=$(shell $(build)/gext.test) $< -o $@

$(build)/gext.test: $(src)/gext.cc $(src)/gextbad.sh
	$(CXX) $(CXXFLAGS) $< -o $@ || cp $(src)/gextbad.sh $@

$(build)/re2g_usage.h: $(src)/usage
	od -b -A n $(src)/usage|xargs -J % echo '#define RE2G_USAGE_STR {' % '0}'|sed 's/\([0-9]\{1,\}\) /0\1, /g' > $@

test: $(build)/re2g $(tests)/tests.sh
	$(tests)/tests.sh $(build)/re2g

clean:
	$(RM) ./$(build)/*
