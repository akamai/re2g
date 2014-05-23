CXX?=g++

re2g: src/re2g.cxx build/re2g_usage.h
#	$(CXX) -D "RE2G_USAGE_STR={0$(shell echo `od -b -A n src/usage`|sed 's/ /,0/g')}"  -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/opt/re2/lib/ src/*.cxx -o re2g
	$(CXX) -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/opt/re2/lib/ src/*.cxx -o re2g

build/re2g_usage.h: src/usage
	echo "#define RE2G_USAGE_STR {0$(shell echo `od -b -A n src/usage`|sed 's/ /,0/g')}" > $@

test: re2g tests/test.sh
	tests/test.sh ./re2g


