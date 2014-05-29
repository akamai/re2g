#include <re2/re2.h>
#include <stdio.h>

int main(int c, char** v){
  std::string out;
  RE2::GlobalExtract("0food1","[0-9]","\\0",&out);
  printf("%s\n",out.c_str());
  return 0;
}

