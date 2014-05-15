#include <re2/re2.h>
#include <iostream>
#include <fstream>

int extract(const re2::StringPiece &text,
            const re2::RE2& pattern,
            const re2::StringPiece &rewrite,
            std::string *out,
            bool global){
  return global?RE2::GlobalExtract(text, pattern, rewrite, out):
    RE2::Extract(text, pattern, rewrite, out)?1:0;
}


int replace(std::string *text,
            const re2::RE2& pattern,
            const re2::StringPiece &rewrite,
            bool global){
  return global?RE2::GlobalReplace(text, pattern, rewrite):
    RE2::Replace(text, pattern, rewrite)?1:0;
}

int main(int argc, char** argv){
  int o_global=0,
    o_usage=0,
    o_print_match=0,
    o_also_print_unreplaced=0,
    o_negate_match=0,
    o_substitute,
    o_print_fname;
  enum {SEARCH,REPLACE} mode;
  int files_arg;

  if(argc>1){
    if(argv[1][0] == '-'){
      if ( argv[1][1] == '-'){
        //ignore standalone '--' as argv[1];
      } else {
        // we have flags
        char* fs=argv[1]+1;
        char c;
        while((c=*fs++)){
          switch (c) {
          case 'g':
            o_global = 1;
            break;
          case 'o':
            o_print_match = 1;
            break;
          case 'p':
            o_also_print_unreplaced = 1;
            break;
          case 'v':
            o_negate_match = 1;
            break;
          case 's':
            o_substitute = 1;
            break;
          case 'h':
            o_print_fname = 1;
            break;
          default:
            o_usage = 1;
          }
        }
      }
      argv[1] = argv[0];
      argv++;
      argc--;
    }
  } else {
    o_usage = 1;
  }


  if(!o_substitute && argc >= 3){
    mode = SEARCH;
    files_arg= 2;
  } else if(o_substitute && argc >= 4){
    mode = REPLACE;
    files_arg = 3;
  } else {
    o_usage = 1;
  }


  if(o_usage){
    std::cout << argv[0] << " [-flags] pattern [replacement] file1..." << std::endl
              << std:: endl
              << "TEXT text to search" << std::endl
              << "PATTERN re2 expression to apply" << std::endl
              << "REPLACEMENT optional replacement string, supports \\0 .. \\9 references"  << std::endl
              << "FLAGS modifiers to operation"  << std::endl
              << std::endl
              << "   h: display help"  << std::endl
              << "   v: invert match"  << std::endl
              << "   o: only print matching portion"  << std::endl
              << "   s: do substitution"  << std::endl
              << "   g: global search/replace, default is one per line"  << std::endl
              << "   p: (when replacing) print lines where no replacement was made"  << std::endl
              << "   h: always print file name"  << std::endl
              << std::endl;
    return 0;
  }

  /*
  std::cout << "g:" << o_global << std::endl << 
    "u:" << o_usage << std::endl <<
    "p:" << o_print_match << std::endl <<
    "u:" << o_also_print_unreplaced << std::endl <<
    "n:" << o_negate_match << std::endl;*/

  std::string out;
  std::string *to_print = NULL;
  RE2::RE2 pat(argv[1]);

  //rationalize flags
  if(o_negate_match && o_print_match){
    o_print_match = 0;
  }

  bool matched;
  bool print;
  std::string line;
  std::string rep;

  if(mode == REPLACE) {
    rep = std::string(argv[2]);
  } else if(mode == SEARCH && o_print_match){
    //o_print_match for SEARCH uses REPLACE code with constant repstr
    rep = std::string("\\0");
    o_also_print_unreplaced = 0;
    mode = REPLACE;
  } 
  if(files_arg < argc - 1){
    o_print_fname = 1;
  }
  while(files_arg < argc){
    char* fname = argv[files_arg];
    std::ifstream ins(fname);

    while (std::getline(ins, line)) {
      std::string in(line);
      
      if(mode == SEARCH){
        // re2g str pat
        matched = RE2::PartialMatch(in, pat);
        to_print = &in;
        to_print=(matched ^ o_negate_match)?to_print:NULL;
      } else if(mode == REPLACE) {
      // re2g str pat rep
        
      // need to pick: (-o) Extract, (default)Replace, (-g)GlobalReplace
      // also, print non matching lines? (-p)
        if(o_print_match){
          matched = extract(in, pat, rep, &out, o_global) > 0;
          to_print = &out;
        } else {
          matched = replace(&in, pat, rep, o_global) > 0;
          to_print = &in;
        }
        to_print=(matched ^ o_negate_match)?to_print:NULL;
        
        if(o_also_print_unreplaced && !to_print){
          out = std::string(line);
          to_print = & out;
        }
      }
      if(to_print){
        if(o_print_fname){
          std::cout << fname << ":";
        }
        std::cout << *to_print << std::endl;
      }
    }
    ins.close();
    files_arg++;
  }
}

