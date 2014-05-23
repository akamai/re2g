CXX?=g++

re2g: src/re2g.cxx src/usage
	$(CXX) -D "RE2G_USAGE_STR=$(shell echo \{0`od -b -A n src/usage`\}|sed  's/ /,0/g')"  -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/opt/re2/lib/ src/*.cxx -o re2g
#	$(CXX) -D "RE2G_USAGE_STR=\"`while IFS='' read l; do printf '%s\\\n' "$$l";done < src/usage | sed s/\\\"/\\\\\\\\\\"/g`\""  -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/opt/re2/lib/ src/*.cxx -o re2g



test: re2g tests/test.sh
	tests/test.sh ./re2g


