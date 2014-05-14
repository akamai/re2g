CXX?=g++

re2g: src/re2g.cxx
	$(CXX)  -lre2 -I/opt/re2/include/ -L/opt/re2/lib/ src/*.cxx -o re2g

test: re2g tests/test.sh
	tests/test.sh ./re2g


