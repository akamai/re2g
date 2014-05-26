CXX?=g++

#HAVE_GLOBAL_EXTRACT=$(shell echo '#include <re2/re2.h>\nint main(int c,char** v){std::string out;RE2::GlobalExtract("food","o","$$1",&out);std::printf("%s\\n",out.c_str()); return 0;}' | $(CXX) -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/opt/re2/lib/ -x c++ - -o build/gext.test 2>/dev/null && echo ZZ$?; )

#HAVE_GLOBAL_EXTRACT=$(shell echo 'int main(int c,char** v){std::string out;/*RE2::GlobalExtract("food","o","$$1",&out);std::printf("%s\\n",out.c_str());*/ return 0;}' | $(CXX) -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/Users/ekobrin/Downloads/re2/obj/ -x c++ - -o build/gext.test -include '/opt/re2/include/re2/re2.h' ; echo foo)

build/re2g: src/re2g.cxx build/re2g_usage.h build/gext.test
#	$(CXX) -D "RE2G_USAGE_STR={0$(shell echo `od -b -A n src/usage`|sed 's/ /,0/g')}"  -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/opt/re2/lib/ src/*.cxx -o re2g
	$(CXX) -D HAVE_GLOBAL_EXTRACT=$(shell build/gext.test) -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/opt/re2/lib/ src/re2g.cxx -o $@

build/gext.test: src/gext.cc src/gextbad.sh
#	$(CXX) $< -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/Users/ekobrin/Downloads/re2/obj/ -o $@ || cp src/gextbad.sh $@
	$(CXX) $< -Ibuild -Wall -lre2 -I/opt/re2/include/ -L/opt/re2/lib -o $@

build/re2g_usage.h: src/usage
#	echo  "#define RE2G_USAGE_STR {0}" > $@;
	echo "#define RE2G_USAGE_STR {0$(shell echo `od -b -A n src/usage`|sed 's/ /,0/g')}" > $@


test: build/re2g tests/tests.sh
	tests/tests.sh build/re2g

clean:
	$(RM) build/*
