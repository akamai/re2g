#include <re2/re2.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <getopt.h>
#include <deque>

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

struct prefix {
  const char* fname;
  int line_no;
  int offset;
};

bool match(const re2::StringPiece &text,
           const re2::RE2& pattern,
           bool whole_line) {
  return whole_line ? RE2::FullMatch(text, pattern) :
    RE2::PartialMatch(text, pattern);
}

int str_to_size(const char* str){
  int v = 0;
  if(str){
    v = atoi(str);
  }
  if(v < 0) {
    v = 0;
  }
  return v;
}

void emit_line(const struct prefix *prefix,char marker, const std::string s){
  if(prefix->fname){
    std::cout << prefix->fname << marker;
  }
  if(prefix->line_no >= 0){
    std::cout << prefix->line_no << marker;
  }
  if(prefix->offset >= 0){
    std::cout << prefix->offset << marker;
  }
  std::cout << s << std::endl;
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
    o_full_line = 0,
    o_after_context = 0,
    o_before_context = 0,
    o_print_lineno = 0,
    o_max_matches = 0;
  enum {SEARCH, REPLACE} mode;

  const struct option options[] = {
    {"help",no_argument,&o_usage,'?'},
    {"global",no_argument,&o_global,'g'},
    {"invert-match",no_argument,&o_negate_match,'v'},
    {"only-matching",no_argument,&o_global,'o'},
    {"subtitute",required_argument,&o_substitute,'s'},
    {"print-all",no_argument,&o_also_print_unreplaced,'p'},
    {"print-names",no_argument,&o_print_fname,'H'},
    {"no-print-names",no_argument,&o_no_print_fname,'h'},
    {"count",no_argument,&o_count,'c'},
    {"files-with-matches",no_argument,&o_list,'l'},
    {"files-without-match",no_argument,&o_neg_list,'L'},
    {"ignore-case",no_argument,&o_case_insensitive,'i'},
    {"fixed-strings",no_argument,&o_literal,'F'},
    {"line-regexp",no_argument,&o_full_line,'x'},
    {"after-context",optional_argument,NULL,'A'},
    {"before-context",optional_argument,NULL,'B'},
    {"context",optional_argument,NULL,'C'},
    {"line-number",no_argument,NULL,'n'},
    {"max-count",required_argument,&o_max_matches,'m'},
    { NULL, 0, NULL, 0 }
  };

  std::string rep;
  char c;
  int longopt=0;
  while((c = getopt_long(argc, (char *const *)argv, "?ogvgs:pHhclLiFxB:C:A:nm:",
                         (const struct option *)&options[0], &longopt))!=-1){
    if(0 == c && longopt >= 0 && 
       longopt < sizeof(options) - 1){
      c = options[longopt].val;
    }
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
      rep=std::string(optarg);
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
    case 'A':
      o_after_context = str_to_size(optarg);
      break;
    case 'B':
      o_before_context = str_to_size(optarg);
      break;
    case 'C':
      o_after_context = o_before_context = str_to_size(optarg);
      break; 
    case 'm':
      o_max_matches = str_to_size(optarg);
      break; 
    case 'n':
      o_print_lineno = 1;
      break; 
    default:
      o_usage = 1;
    }
  }
  argc -= optind;
  argv += optind;
  
  if(o_neg_list){
    o_list = 1;
  }

  /*  
  for(const struct option *o=&options[0];o->name;o++){
    std::cout << o->name << ':';
    if(o->flag){
      std::cout << *o->flag;
    } else {
      std::cout << "NULL";
    }
    std::cout << std::endl;
  }
  
  std::cout << "C: [" << o_after_context << ',' << o_before_context << ']' << std::endl;

  std::cout << "ARGC: " << argc << std::endl;
  for(int i=0;i<argc;i++){
    std::cout << "ARGV[" << i << "]: " << argv[i]  << std::endl;
  }
  std::cout   << std::endl;
  */
  if(argc >= 1){
    mode=o_substitute?REPLACE:SEARCH;
  } else {
    o_usage = 1;
  }


  if(o_usage) {
    std::cout << appname << " [-?ogvgpHhclLiFxBACnm][-s substitution] pattern file1..." << std::endl
              << std:: endl
              << "PATTERN re2 expression to apply" << std::endl
              << std::endl
              << "   -?, --help: display help"  << std::endl
              << "   -v, --invert-match: invert match"  << std::endl
              << "   -o, --only-matching: only print matching portion"  << std::endl
              << "   -s substitution, --sub=substitution optional replacement string, supports \\0 .. \\9 references"  << std::endl
              << "   -g, --global: global search/replace, default is one per line"  << std::endl
              << "   -p, --print-all: (when replacing) print lines where no replacement was made"  << std::endl
              << "   -H, --print-names: always print file name"  << std::endl
              << "   -h, --no-print-names: never print file name"  << std::endl
              << "   -c, --count: print match count instead of normal output"  << std::endl
              << "   -l, --files-with-matches: list matching files instead of normal output"  << std::endl
              << "   -L, --files-without-match: list nonmatching files instead of normal output"  << std::endl
              << "   -i, --ignore-case: ignore case when matching; same as (?i)"  << std::endl
              << "   -F, --fixed-strings: treat pattern argument as literal string"  << std::endl
              << "   -x, --line-regexp: match whole lines only"  << std::endl
              << "   -B num, --before-context=num Display num lines preceding any match"  << std::endl
              << "   -A num, --after-context=num Display num lines following any match"  << std::endl
              << "   -C num, --context=num same as -A num -B num"  << std::endl
              << "   -n, --line-number print input line numbers, starting at 1"  << std::endl
              << "   -m num, --max-count num stop reading each file after num matches"  << std::endl


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
  if(o_negate_match) {
    o_print_match = 0;
  }

  if(o_count || mode == SEARCH) {
    o_also_print_unreplaced = 0;
  }

  if(o_also_print_unreplaced) {
    o_before_context = 0;
    o_after_context = 0;
  }


  if(mode == SEARCH && o_print_match) {
    //o_print_match for SEARCH uses REPLACE code with constant repstr
    rep = std::string("\\0");
    o_also_print_unreplaced = 0;
    mode = REPLACE;
  }

  int num_files = argc - 1;

  if(num_files > 1 && !o_no_print_fname) {
    o_print_fname = 1;
  }

  const char** fnames;
  const char* def_fname = "/dev/stdin";

  if(num_files == 0) {
    num_files = 1;
    fnames = &def_fname;
  } else {
    fnames = &argv[1];
  }

  for(int fidx = 0; fidx < num_files; fidx++) {
    const char* fname = fnames[fidx];
    std::ifstream ins(fname);
    std::deque<std::string> before(o_before_context);
    before.clear();
    if(fnames == &def_fname) {
      //for output compatibility with grep
      fname = "(standard input)";
    }

    if(!ins) {
      std::cerr << appname << " " << fname << ':' << strerror(errno) << std::endl;
    } else {
      long long count = 0;
      std::string line;
      int ca_printed = o_after_context;
      struct prefix pref = {
        o_print_fname?fname:NULL,
        o_print_lineno?1:-1,
        -1
      };

      while(std::getline(ins, line) && (!o_max_matches || count < o_max_matches)) {
        //std::cout << before.size() << std::endl;
        std::string *to_print = NULL;
        std::string in(line);
        std::string out;
        bool matched = false;

        if(mode == SEARCH) {
          matched = o_negate_match ^ match(in, pat, o_full_line);
          to_print = &in;
        } else if(mode == REPLACE) {
          // need to pick: (-o) Extract, (default) Replace, (-g)GlobalReplace
          // also, print non matching lines? (-p)
          
          if(o_print_match) {
            matched = o_negate_match ^ (extract(in, pat, rep, &out, o_global) > 0);
            to_print = &out;
          } else {
            matched = o_negate_match ^ (replace(&in, pat, rep, o_global) > 0);
            to_print = &in;
          }
          
          if(o_also_print_unreplaced && !matched) {
            to_print = &line;
          }
        }
        if(!(matched || o_also_print_unreplaced)){
          to_print = NULL;
        }
        if(matched){
          count++;
          ca_printed = 0;
          if(o_before_context){
            pref.line_no-=before.size();
            while(!before.empty()){
              emit_line(&pref,'-',before.front());
              before.pop_front();
              pref.line_no++;
            }
          }
        }
        if(ca_printed < o_after_context && !to_print && !o_count){
            to_print = &line;
            ca_printed ++;
        }
        if(to_print) {
          if(!o_count && !o_list) {
            emit_line(&pref,matched?':':'-',*to_print);
          }
        } else {
          if(o_before_context > 0){
            if(before.size() >= o_before_context){
              before.pop_front();
            }
            before.push_back(std::string(line));
          }
        }
        if(pref.line_no>=0){
          pref.line_no++;
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

