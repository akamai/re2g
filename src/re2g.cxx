 /* 
    Hello code reader. Welcome to my re2-based grep-alike
    
    I've been wanting to experiment with RE2 so that I could better speak
    about it when discussing things like PCRE usage in other software. I
    didn't quickly find a grep-like tool.

    So I wrote one.

    Then I got carried away matching the grep options listed in the man page

    This is the state of this project:

    Known issues:
      1) This depends on my patch to RE2 to enable GlobalExtract;
         see: https://groups.google.com/d/msg/re2-dev/uI-9maDcUVw/TiRXXNpsEZwJ
      2) Error reporting is haphazard. Sorry. Patches welcome.
      3) There are probably be memory leaks, etc. It's C++ code
         and I don't usually write C++.
      4) The test suite stinks. It's a crummy shell script.

     Differences from grep:
      1) Locale environment variables are ignored, as are GREP_*
      2) Our -s is substitution, not silence. It's a good character for s///.
         There is no option for silence. Redirectind stderr is close.
      3) Multiple patterns via -e or -f differs:
          Ordering of output is different
          Combining with -o: output is different from grep, arguably better
          Combining with -n: output is different from grep, arguably better
      4) Every long option has a short version and vice-versa
      5) -C must take a parameter. This differs from grep's
         documentation, but not behavior.
      6) The -Z, -z, -J options are implemented via the -X option, we do not
         link against any of the compression libraries.
      7) There is no BRE support
      8) POSIX mode in RE2 seems stricter than in grep
      9) Byte-offset reporting is not supported
     10) directory recursion is not supported, nor are the associated options.
         If you need recursion, please use find and xargs.
     11) No fancy color options. You can try to use -s and ANSI sequences.
   */


#include <re2/re2.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <getopt.h>
#include <deque>
#include <unistd.h>

#include <streambuf>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <sys/wait.h>

class fdbuf : public std::streambuf {
public:
  explicit fdbuf(int fd, std::size_t buff_sz = 4096, std::size_t put_back = 128);
  ~fdbuf();
  
private:
  // overrides base class underflow()
  std::streambuf::int_type underflow();
  
  // copy ctor and assignment not implemented;
  // copying not allowed
  fdbuf(const fdbuf &);
  fdbuf &operator= (const fdbuf &);
  
private:
  int fd_;
  std::size_t put_back_;
  int buff_sz_;
  int full_sz_;
  char* start_;
  char* base_;
  char* end_;
};

fdbuf::fdbuf(int fd, std::size_t buff_sz, std::size_t put_back) :
  fd_(fd),
  put_back_(std::max(put_back, size_t(1))),
  buff_sz_(std::max(buff_sz, put_back_)),
  full_sz_(buff_sz_ + put_back_),
  start_(new char[full_sz_]),
  base_(start_ + put_back_),
  end_(start_ + full_sz_)
{
  setg(end_,end_,end_);
}

fdbuf::~fdbuf() {
  delete[] start_;
}

std::streambuf::int_type fdbuf::underflow(){
  if(gptr() >= egptr()){
    int backed = std::min((std::size_t)(gptr() - eback()), put_back_);
    if(backed > 0){
      //std::cerr << "backed: " << backed << std::endl;
      std::memcpy(base_ - backed, gptr()-backed, backed);
    }
    int qty = read(fd_, base_, buff_sz_);
    //std::cerr << "qty: " << qty << std::endl;
    if(qty < 0){
      //std::cerr << "err: " <<  strerror(errno) << std::endl;
      return traits_type::eof();
    }
    if(qty == 0){
      //std::cerr << "EOF" <<std::endl;
      return traits_type::eof();
    }
    setg(start_, base_, base_ + qty);
  }
  return traits_type::to_int_type(*gptr());
}


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

void emit_line(const struct prefix *prefix, char marker, const std::string s,
               const std::string eol, bool flush_after){
  if(prefix->fname){
    std::cout << prefix->fname << marker;
  }
  if(prefix->line_no >= 0){
    std::cout << prefix->line_no << marker;
  }
  if(prefix->offset >= 0){
    std::cout << prefix->offset << marker;
  }
  std::cout << s << eol;
  if(flush_after){
    std::cout.flush();
  }
}

enum input_type {STDIN,CAT,NAME};

fdbuf *ioexec(char* const* arglist, const char* fname, enum input_type util_input){
  int plumb[2];
  pid_t pid;

  if(pipe(&plumb[0])){
    std::cerr << "error piping for " << arglist[0] << ": "
              << strerror(errno) << std::endl;
    return NULL;
  }
  pid = fork();
  if(pid < 0){
    std::cerr << "error forking for  " << arglist[0] << ": "
              << strerror(errno) << std::endl;
    return NULL;
  }
  if(!pid){
    if(util_input != STDIN){
      close(STDIN_FILENO);
    }

    if(util_input == CAT){
      int ifd = open(fname, O_RDONLY);
      if(ifd <0 ){
        std::cerr << "error opening " << fname << ": "
                  << strerror(errno) << std::endl;
        std::exit(-1);
      }
      if(dup2(ifd, STDIN_FILENO) != STDIN_FILENO){
        std::cerr << "error dup2ing for input " << arglist[0] << ": "
                  << strerror(errno) << std::endl;
        std::exit(-2);
      }
    }
    close(STDOUT_FILENO);
    fcntl(STDERR_FILENO, F_SETFD, 1);
    close(plumb[0]);
    if(dup2(plumb[1], STDOUT_FILENO) != STDOUT_FILENO){
      std::cerr << "error dup2ing for output " << arglist[0] << ": "
                << strerror(errno) << std::endl;
      std::exit(-3);
    }
    
    execvp(arglist[0],&arglist[0]);
    std::cerr << "error invoking  " << arglist[0] << ": "
              << strerror(errno) << std::endl;
    std::exit(-4);
  }

  if(util_input == STDIN){
    close(STDIN_FILENO);
  }
  close(plumb[1]);
  return new fdbuf(plumb[0]);
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
    o_max_matches = 0,
    o_quiet_and_quick = 0,
    o_special_delimiter = 0,
    o_posix_extended_syntax = 0,
    o_pat_str = 0,
    o_pat_file = 0,
    o_line_buffered = isatty(STDOUT_FILENO);
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
    {"after-context",required_argument,NULL,'A'},
    {"before-context",required_argument,NULL,'B'},
    {"context",optional_argument,NULL,'C'},
    {"line-number",no_argument,NULL,'n'},
    {"max-count",required_argument,&o_max_matches,'m'},
    {"exec",required_argument,NULL,'X'},
    {"quiet",no_argument,&o_quiet_and_quick,'q'},
    {"silent",no_argument,&o_quiet_and_quick,'q'},
    {"null",no_argument,&o_special_delimiter,'0'},
    {"line-buffered",no_argument,&o_line_buffered,'N'},
    {"decompress",no_argument,&o_line_buffered,'Z'},
    {"gzdecompress",no_argument,&o_line_buffered,'z'},
    {"bz2decompress",no_argument,&o_line_buffered,'J'},
    {"extended-regexp",no_argument,&o_posix_extended_syntax,'E'},
    {"regexp",required_argument,&o_pat_str,'e'},
    {"file",required_argument,&o_pat_file,'f'},
    { NULL, 0, NULL, 0 }
  };

  std::string rep;
  std::deque<std::string> pat_strs;
  std::deque<std::string> pat_files;
  std::string eol("\n");
  std::deque<std::string> uargs(0);
  char c;
  int longopt = 0;
  while((c = getopt_long(argc, (char *const *)argv, "?ogvgs:pHhclLiFxB:C:A:nm:X:qN0zZJEe:f:",
                         (const struct option *)&options[0], &longopt))!=-1){
    if(0 == c && longopt >= 0 && 
       longopt < sizeof(options) - 1){
      c = options[longopt].val;
    }
    switch(c) {
    case '0':
      o_special_delimiter = 1;
      if(optarg){
        eol = std::string(optarg);
      } else {
        eol = std::string("\0");
      }
      break;
    case 'N':
      o_line_buffered = 1;
      break;
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
      o_after_context = o_before_context = optarg?str_to_size(optarg):2;
      break; 
    case 'm':
      o_max_matches = str_to_size(optarg);
      break;
    case 'Z':
      uargs.clear();
      uargs.push_back("zcat");
      break;
    case 'z':
      uargs.clear();
      uargs.push_back("gzcat");
      break;
    case 'J':
      uargs.clear();
      uargs.push_back("bzcat");
      break;
    case 'X':
      uargs.clear();
      // consume until arg is a single semi-colon like find -exec
      const char* oa;
      for(oa = optarg;
          optind < argc
            && oa
            && ! (oa[0] == ';' && oa[1] == 0) ;
          oa=argv[optind++]){
        uargs.push_back(oa);
      }
      if(!(oa[0] == ';' && oa[1] == 0)){
        o_usage = 1;
      }
      break;
    case 'n':
      o_print_lineno = 1;
      break;
    case 'q':
      o_quiet_and_quick = 1;
      break; 
    case 'E':
      o_posix_extended_syntax = 1;
      break; 
    case 'e':
      o_pat_str++;
      pat_strs.push_back(std::string(optarg));
      break;
    case 'f':
      o_pat_file++;
      pat_files.push_back(std::string(optarg));
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
  if(argc >= 1 || o_pat_str || o_pat_file){
    mode=o_substitute?REPLACE:SEARCH;
  } else {
    o_usage = 1;
  }

  if(o_usage) {
    std::cout << appname << " [-?ogvgpHhclLiFxBACnmq0NzZJE] [-f file][-X utility ...] [-s substitution] [-e] pattern file1..." << std::endl
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
              << "   -C num, --context[=num] same as -A num -B num, long-form defaults to 2"  << std::endl
              << "   -n, --line-number print input line numbers, starting at 1"  << std::endl
              << "   -m num, --max-count num stop reading each file after num matches"  << std::endl
              << "   -X utility [argument ...] ; , --exec=utility [argument ...] ;  Invokes utility on each input file or stdin, using syntax much like find. The invocation replaces instances of '{}' with the name of the current file. If no '{}' appears, then the file contents will be passed as standard input to the utility. The trailing semicolon is mandatory. Uses execvep."  << std::endl
              << "   -q, --quiet, --silent suppress normal output, just emit results via exit code: 0 => no match, 1 => at least one match in at least one input. Stops as soon as a match is found in any input."  << std::endl
              << "   -0, --null=[other] separate lines of output with the null character or specified other string, useful with -l and pipes to xargs"  << std::endl
              << "   -N, --line-buffered  flush after each line, even if stdout is not a tty"  << std::endl
              << "   -Z, --decompress  same as -X zcat \\; NOTE: zcat must be in $PATH"  << std::endl
              << "   -z, --gzdecompress  same as -X gzcat \\; NOTE: gzcat must be in $PATH"  << std::endl
              << "   -J, --bz2decompress  same as -X bzcat \\; NOTE: bzcat must be in $PATH"  << std::endl
              << "   -E, --extended-regexp  Use Posix Extended Syntax like egrep, this is actually less powerful than the default RE2 expression language"  << std::endl
              << "   -e pattern, --regexp=pattern  Use pattern as regular expression, useful if pattern could be confused with flags. NOTE: differs from POSIX grep in that the last -e pattern is used. Multiple patterns are not supported at this time, construct compound patterns using '|' instead"  << std::endl
              << "   -f file, -file=file Acts as if eachline in 'file' were passed as an argument to the -e option. In other words, the last line of the file will supply the search pattern, until multiple -e options are supported."  << std::endl

              << std::endl;
    return -1;
  }

  re2::RE2::Options opts(re2::RE2::DefaultOptions);

  if(o_case_insensitive) {
    opts.set_case_sensitive(false);
  }
  if(o_literal) {
    opts.set_literal(true);
  }
  if(o_posix_extended_syntax) {
    opts.set_posix_syntax(true);
  }

  std::deque<RE2::RE2*> pats;

  while(!pat_files.empty()){
    std::ifstream patf(pat_files.front().c_str());
    if(!patf){
      std::cerr << appname << ": " << pat_files.front() << ": " << strerror(errno) << std::endl;
      return -1;
    }
    std::string pat_str;
    while(std::getline(patf, pat_str)) {
      pat_strs.push_back(pat_str);
    }
    pat_files.pop_front();
  }
  if(!o_pat_str && !o_pat_file){
    pat_strs.push_back(argv[0]);
    argv++;
    argc--;
  }

  while(!pat_strs.empty()){
    pats.push_back(new RE2::RE2(pat_strs.front(),opts));
    pat_strs.pop_front();
  }


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

  int num_files = argc;

  if(num_files > 1 && !o_no_print_fname) {
    o_print_fname = 1;
  }

  const char** fnames;
  const char* def_fname = "-";
  bool using_stdin;
  if(num_files == 0) {
    num_files = 1;
    using_stdin = true;
    fnames = &def_fname;
  } else {
    using_stdin = false;
    fnames = &argv[0];
  }

  const char **uargv = NULL;
  int uargc = uargs.size();
  int rargc = 0;
  RE2::RE2 uarg_pat("\\{\\}");
  if(!uargs.empty()){
    uargv = new const char*[uargc + 1];
    uargv[uargc] = NULL;
    for(int aidx = 0; aidx < uargc; aidx++){
      std::string *arg = &uargs[aidx];
      if(match(*arg, uarg_pat, false)){
        uargv[aidx]=NULL;
        rargc++;
      } else {
        uargv[aidx]=arg->c_str();
      }
    }
  }
  std::deque<std::string> rargs(rargc);
  int retval = 1;
  for(int fidx = 0; fidx < num_files; fidx++) {
    const char* fname = fnames[fidx];
    if(fname[0]=='-' && fname[1] == 0){
      fname = def_fname;
      using_stdin = true;
    }
    std::istream *is = NULL;
    fdbuf *pb = NULL;
    std::filebuf *fb = NULL;
    if(uargv){
      int ridx = 0;
      //      std::cout << "fork: ";
      for(int aidx = 0; aidx < uargc; aidx++){
        if(uargv[aidx] == NULL){
          rargs[ridx]=std::string(uargs[aidx]);
          replace(&rargs[ridx], uarg_pat, fname, true);
          uargv[aidx]=rargs[ridx].c_str();
          ridx++;
        }
        //        std::cout << "   " << uargv[aidx] << ',';
      }
      enum input_type util_input;
      if(rargc){
        util_input = NAME;
      } else {
        util_input = using_stdin?STDIN:CAT;
      }
      
      pb = ioexec((char *const *)uargv, fname, util_input);
      if(!pb){
        std::cerr << appname << ": failed to use utility: " << strerror(errno) << std::endl;
        return -1;
      }
      is = new std::istream(pb);
    } else if(using_stdin){
      is = &std::cin;
    } else {
      fb = new std::filebuf();
      if(fb->open(fname, std::ios_base::in)){
        is = new std::istream(fb);
      }
    }

    std::deque<std::string> before(o_before_context);
    before.clear();
    if(using_stdin) {
      //for output compatibility with grep
      fname = "(standard input)";
    }

    if(!is) {
      std::cerr << appname << ": " << fname << ": " << strerror(errno) << std::endl;
    } else {
      long long count = 0;
      std::string line;
      int ca_printed = o_after_context;
      struct prefix pref = {
        o_print_fname?fname:NULL,
        o_print_lineno?1:-1,
        -1
      };
      while(std::getline(*is, line) && (!o_max_matches || count < o_max_matches)) {
        //std::cout << before.size() << std::endl;
        std::string *to_print = NULL;
        std::string in(line);
        std::string out;
        std::string obuf;
        int num_pats_matched = 0;
        obuf.clear();
        for(std::deque<RE2::RE2*>::iterator pat = pats.begin();
            pat != pats.end();
            ++pat){

          bool this_pat_matched = false;

          if(mode == SEARCH) {
            this_pat_matched = o_negate_match ^ match(in, **pat, o_full_line);
            to_print = &in;
          } else if(mode == REPLACE) {
            // need to pick: (-o) Extract, (default) Replace, (-g)GlobalReplace
            // also, print non matching lines? (-p)
            
            if(o_print_match) {
              this_pat_matched = o_negate_match ^ (extract(in, **pat, rep, &out, o_global) > 0);
              to_print = &out;
            } else {
              this_pat_matched = o_negate_match ^ (replace(&in, **pat, rep, o_global) > 0);
              to_print = &in;
            }
            
            if(o_also_print_unreplaced && !this_pat_matched) {
              to_print = &line;
            }
          }
          if(this_pat_matched){
            num_pats_matched++;
            if(mode == REPLACE) {
              if(!obuf.empty()){
                obuf += '\n'; //TODO: use configurable line ending
              }
              obuf += *to_print;
            }
          }
        }
        bool any_pat_matched = num_pats_matched > 0;
        if(any_pat_matched && mode == REPLACE){
          to_print = &obuf;
        }
        if(!(any_pat_matched || o_also_print_unreplaced)){
          to_print = NULL;
        }
        if(any_pat_matched){
          if(o_quiet_and_quick){
            return 0;
          }
          count++;
          ca_printed = 0;
          if(o_before_context){
            pref.line_no-=before.size();
            while(!before.empty()){
              emit_line(&pref,'-',before.front(),eol,o_line_buffered);
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
            emit_line(&pref,any_pat_matched?':':'-',*to_print,eol,o_line_buffered);
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
      if(count > 0 ){
        retval = 0;
      }
      if(o_count) {
        if(o_print_fname) {
          std::cout << fname << ":";
        }
        std::cout << count << eol;
        if(o_line_buffered){
          std::cout.flush();
        }
      } else if(o_list && (count > 0) ^ o_neg_list) {
        std::cout << fname << eol;
        if(o_line_buffered){
          std::cout.flush();
        }
      }
      if(using_stdin){
        using_stdin = false;
      } else {
        delete is;
      }
    }
    if(pb){
      int sl;
      wait(&sl);
      delete pb;
    }
    if(fb){
      delete fb;
    }
  }

  while(!pats.empty()) {
    delete pats.back();
    pats.pop_back();
  }

  if(uargv){
    delete[] uargv;
  }
  return retval;
}

