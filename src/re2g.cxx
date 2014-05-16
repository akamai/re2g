#include <re2/re2.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <getopt.h>

int extract(const re2::StringPiece &text,
            const re2::RE2& pattern,
            const re2::StringPiece &rewrite,
            std::string *out,
            bool global) {
  return global ? RE2::GlobalExtract(text, pattern, rewrite, out) :
    RE2::Extract(text, pattern, rewrite, out) ? 1 : 0;
}


int replace(std::string *text,
            const re2::RE2& pattern,
            const re2::StringPiece &rewrite,
            bool global) {
  return global ? RE2::GlobalReplace(text, pattern, rewrite) :
    RE2::Replace(text, pattern, rewrite) ? 1 : 0;
}

bool match(const re2::StringPiece &text,
           const re2::RE2& pattern,
           bool whole_line) {
  return whole_line ? RE2::FullMatch(text, pattern) :
    RE2::PartialMatch(text, pattern);
}

int main(int argc, const char **argv) {
  const char *appname = argv[0];
  const char *apn = argv[0];
  while(*apn) {
    if(*apn++ == '/') {
      appname = apn;
    }
  }

  int o_global = 0,
    o_usage = 0,
    o_print_match = 0,
    o_also_print_unreplaced = 0,
    o_negate_match = 0,
    o_substitute = 0,
    o_print_fname = 0,
    o_no_print_fname = 0,
    o_count = 0,
    o_list = 0,
    o_neg_list = 0,
    o_literal = 0,
    o_case_insensitive = 0,
    o_full_line = 0;
  enum {SEARCH, REPLACE} mode;

  const struct option options[] = {
    {"help",no_argument,&o_usage,'?'},
    {"global",no_argument,&o_global,'g'},
    {"invert-match",no_argument,&o_negate_match,'v'},
    {"only-matching",no_argument,&o_global,'o'},
    {"subtitute",no_argument,&o_substitute,'s'},
    {"print-all",no_argument,&o_also_print_unreplaced,'p'},
    {"print-names",no_argument,&o_print_fname,'H'},
    {"no-print-names",no_argument,&o_no_print_fname,'h'},
    {"count",no_argument,&o_count,'c'},
    {"files-with-matches",no_argument,&o_list,'l'},
    {"files-without-match",no_argument,&o_neg_list,'L'},
    {"ignore-case",no_argument,&o_case_insensitive,'i'},
    {"fixed-strings",no_argument,&o_literal,'F'},
    {"line-regexp",no_argument,&o_full_line,'x'},
    { NULL, 0, NULL, 0 }
  };

  char c;
  while((c = getopt_long(argc, (char *const *)argv, "?ogvgspHhclLiFx",
                              (const struct option *)&options[0], NULL))!=-1){
    switch(c) {
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
    case 'H':
      o_print_fname = 1;
      break;
    case 'h':
      o_no_print_fname = 1;
      break;
    case 'c':
      o_count = 1;
      break;
    case 'l':
      o_list = 1;
      break;
    case 'L':
      o_neg_list = 1;
      break;
    case 'F':
      o_literal = 1;
      break;
    case 'i':
    case 'y':
      o_case_insensitive = 1;
      break;
    case 'x':
      o_full_line = 1;
      break;
    default:
      o_usage = 1;
    }
  }
  argc -= optind;
  argv += optind;
  
  //      }
  //      argv[1] = argv[0];
  //      argv++;
  //      argc--;
  //}
  // } else {
  //    o_usage = 1;
  //  }
  
  if(o_neg_list){
    o_list = 1;
  }
  
  /*  for(const struct option *o=&options[0];o->name;o++){
    std::cout << o->name << ':' << *(o->flag) << std::endl;
  }
  
  std::cout << "ARGC: " << argc << std::endl;
  for(int i=0;i<argc;i++){
    std::cout << "ARGV[" << i << "]: " << argv[i]  << std::endl;
  }
  std::cout   << std::endl;*/

  int files_arg;
  if(!o_substitute && argc >= 1) {
    mode = SEARCH;
    files_arg = 1;
  } else if(o_substitute && argc >= 2) {
    mode = REPLACE;
    files_arg = 2;
  } else {
    o_usage = 1;
  }


  if(o_usage) {
    std::cout << appname << " [-flags] pattern [replacement] file1..." << std::endl
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
              << "   H: always print file name"  << std::endl
              << "   h: never print file name"  << std::endl
              << "   c: print match count instead of normal output"  << std::endl
              << "   l: list matching files instead of normal output"  << std::endl
              << "   L: list nonmatching files instead of normal output"  << std::endl
              << "   i: ignore case when matching; same as (?i)"  << std::endl
              << "   F: treat pattern argument as literal string"  << std::endl
              << "   x: match whole lines only"  << std::endl
              << std::endl;
    return 0;
  }

  re2::RE2::Options opts(re2::RE2::DefaultOptions);

  if(o_case_insensitive) {
    opts.set_case_sensitive(false);
  }
  if(o_literal) {
    opts.set_literal(true);
  }
  
  RE2::RE2 pat(argv[0], opts);

  //rationalize flags
  if(o_negate_match && o_print_match) {
    o_print_match = 0;
  }

  std::string rep;
  if(mode == REPLACE) {
    rep = std::string(argv[1]);
  } else if(mode == SEARCH && o_print_match) {
    //o_print_match for SEARCH uses REPLACE code with constant repstr
    rep = std::string("\\0");
    o_also_print_unreplaced = 0;
    mode = REPLACE;
  }

  int num_files = argc - files_arg;

  if(num_files > 1 && !o_no_print_fname) {
    o_print_fname = 1;
  }

  const char** fnames;
  const char* def_fname = "/dev/stdin";

  if(num_files == 0) {
    num_files = 1;
    fnames = &def_fname;
  } else {
    fnames = &argv[files_arg];
  }

  for(int fidx = 0; fidx < num_files; fidx++) {
    const char* fname = fnames[fidx];
    std::ifstream ins(fname);

    if(fnames == &def_fname) {
      //for output compatibility with grep
      fname = "(standard input)";
    }

    if(!ins) {
      std::cerr << appname << " " << fname << ':' << strerror(errno) << std::endl;
    } else {
      long long count = 0;
      std::string line;
      while(std::getline(ins, line)) {
        std::string *to_print = NULL;
        std::string in(line);
        bool matched;
        
        if(mode == SEARCH) {
          matched = match(in, pat, o_full_line);
          to_print = &in;
          to_print = (matched ^ o_negate_match) ? to_print : NULL;
        } else if(mode == REPLACE) {
          // need to pick: (-o) Extract, (default) Replace, (-g)GlobalReplace
          // also, print non matching lines? (-p)
          std::string out;
          
          if(o_print_match) {
            matched = extract(in, pat, rep, &out, o_global) > 0;
            to_print = &out;
          } else {
            matched = replace(&in, pat, rep, o_global) > 0;
            to_print = &in;
          }
          to_print = (matched ^ o_negate_match) ? to_print : NULL;
          
          if(o_also_print_unreplaced && !to_print && !o_count) {
            out = std::string(line);
            to_print = &out;
          }
        }
        if(to_print) {
          count++;
          if(!o_count && !o_list) {
            if(o_print_fname) {
              std::cout << fname << ":";
            }
            std::cout << *to_print << std::endl;
          }
        }
      }
      if(o_count) {
        if(o_print_fname) {
          std::cout << fname << ":";
        }
        std::cout << count << std::endl;
        
      } else if(o_list && (count > 0) ^ o_neg_list) {
        std::cout << fname << std::endl;
      }
      ins.close();
    }
  }
}

