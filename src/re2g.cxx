#include <re2/re2.h>
#include <iostream>


#define arraysize(array) (sizeof(array)/sizeof((array)[0]))

bool ExtractAll(const re2::StringPiece &text,
                 const re2::RE2& re,
                 const re2::StringPiece &rewrite,
                std::string *out) {
  re2::StringPiece vec[1234];//[kVecSize];
  int nvec = 1 + RE2::MaxSubmatch(rewrite);
  if (nvec > arraysize(vec))
    return false;

  bool any = false;
  out->clear();
  int consumed = 0;
  while(re.Match(text, consumed, text.size(), RE2::UNANCHORED, vec, nvec)){
    consumed = vec[0].end() - text.begin();
    if(re.Rewrite(out, rewrite, vec, nvec)) {
      any = true;
    }
  }
  return any;
}

int main(int argc,char** argv){
  int o_global=0,
    o_usage=0,
    o_print_match=0,
    o_also_print_unreplaced=0,
    o_negate_match=0;

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
          default:
            o_usage = 1;
          }
        }
      }
      argv[1] = argv[0];
      argv++;
      argc--;
    }
  }

  if(o_usage){
    std::cout << argv[0] << " [-flags] text pattern [replacement]" << std::endl
              << std:: endl
              << "TEXT text to search" << std::endl
              << "PATTERN re2 expression to apply" << std::endl
              << "REPLACEMENT optional replacement string, supports \\0 .. \\9 references"  << std::endl
              << "FLAGS modifiers to operation"  << std::endl
              << std::endl
              << "   h: display help"  << std::endl
              << "   v: invert match"  << std::endl
              << "   o: only print matching portion"  << std::endl
              << "   p: (when replacing) print lines where no replacement was made"  << std::endl
              << "   g: (when replacing) global replace, default is one per line"  << std::endl
         << std::endl;
    return 0;
  }

  /*
  std::cout << "g:" << o_global << std::endl << 
    "u:" << o_usage << std::endl <<
    "p:" << o_print_match << std::endl <<
    "u:" << o_also_print_unreplaced << std::endl <<
    "n:" << o_negate_match << std::endl;*/

  std::string in = std::string(argv[1]);
  std::string out;
  std::string *to_print = NULL;
  RE2::RE2 pat(argv[2]);

  //rationalize flags
  if(o_negate_match && o_print_match){
    o_print_match = 0;
  }

  bool matched;
  bool print;
  if(argc==3){
    // re2g str pat
    // ignores o_global
    if(o_print_match){
      matched = RE2::Extract(in,pat,"\\0",&out);
      to_print=&out;

    } else {
      matched = RE2::PartialMatch(in,pat);
      to_print= &in;
    }
    to_print=(matched^o_negate_match)?to_print:NULL;
  } else if(argc==4) {
    // re2g str pat rep
    // ignores o_print_match

    std::string rep = std::string(argv[3]);

    // need to pick: (-o) Extract, (default)Replace, (-g)GlobalReplace
    // also, print non matching lines? (-p)
    if(o_print_match && !o_global){
      matched = RE2::Extract(in,pat,rep,&out);
      to_print=&out;
    } else if(o_global && !o_print_match) {
      matched = RE2::GlobalReplace(&in,pat,rep);
      to_print=&in;
    } else if(o_global && o_print_match){
      matched = ExtractAll(in,pat,rep,&out);
      to_print=&out;
    } else {
      matched = RE2::Replace(&in,pat,rep);
      to_print=&in;
    }
    to_print=(matched ^ o_negate_match)?to_print:NULL;

    if(o_also_print_unreplaced && !to_print){
      out = std::string(argv[1]);
      to_print = & out;
    }
  } else {
    std::cout << RE2::PartialMatch("f", "f") << std::endl;
    std::cout << RE2::PartialMatch("fred", "z") << std::endl;
    std::cout << RE2::PartialMatch("fred", "[fz]") << std::endl;
  }
  if(to_print){
    std::cout << *to_print << std::endl;
  } else {
    std::cout << std::endl;
  }
}

