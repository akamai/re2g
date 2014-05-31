/*  author: Eric Kobrin <ekobrin@akamai.com>

    Hello code reader. Welcome to my re2-based grep-alike.

    I've been wanting to experiment with RE2 so that I could better speak
    about it when discussing things like PCRE usage in other software. I
    didn't quickly find a grep-like tool which I could use for testing.

    So I wrote one.

    Then I got carried away matching the grep options listed in the man page.

    This tool supports behavior like sed's s/// via -s in addition to find-like
    support for running utilities using -X.

    This is the state of this project:

    Known issues:
      1) Full functionality depends on my patch to RE2 to enable GlobalExtract;
         see: https://groups.google.com/d/msg/re2-dev/uI-9maDcUVw/TiRXXNpsEZwJ
      2) Error reporting is haphazard. Sorry. Patches welcome.
      3) There are probably be memory leaks, etc. It's C++ code
         and I don't usually write C++.
      4) The test suite stinks. It's a crummy shell script that is hard to
         maintain. It also compares this program's behavior to the local grep.

     Differences from grep:
      1) Environment variables are ignored, including the Locale ones in the
         POSIX standard and the GREP_* ones from GNU or BSD.
      2) Our -s is substitution, not silence. It's a good character for s///.
         There is no option for silence. Redirecting stderr is close.
      3) Multiple patterns via -e or -f differs:
          Ordering of output is different
          Combining with -o: output is different from grep, arguably better
          Combining with -n: output is different from grep, arguably better
      4) Every long option has a short version and vice-versa (except --version)
      5) -C must take a parameter. This differs from my local grep's
         documentation, but not its behavior.
      6) The -Z, -z, -J options are implemented via the -X option, we do not
         link against any of the compression libraries.
      7) There is no BRE support
      8) POSIX mode in RE2 seems stricter than in grep
      9) Byte-offset reporting is not supported
     10) Directory recursion is not supported, nor are the associated options.
         If you need recursion, please use find and xargs.
     11) No fancy color options. You can try to use -s and ANSI sequences.
     12) Binary files aren't treated specially, it's like running `grep -a'
     13) When using context, noncontiguous groups of results are not separated
         by lines containing only a pair of dashes.
   */


#include <re2/re2.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <getopt.h>
#include <deque>
#include <unistd.h>
#include <string.h>
#include <streambuf>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define RE2G_VERSION "%s v0.1.34"
#include "re2g_usage.h"

namespace re2g {

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

char USAGE[] = RE2G_USAGE_STR;

fdbuf::fdbuf(int fd, std::size_t buff_sz, std::size_t put_back) :
  fd_(fd),
  put_back_(std::max(put_back, size_t(1))),
  buff_sz_(std::max(buff_sz, put_back_)),
  full_sz_(buff_sz_ + put_back_),
  start_(new char[full_sz_]),
  base_(start_ + put_back_),
  end_(start_ + full_sz_) {
  setg(end_, end_, end_);
}

fdbuf::~fdbuf() {
  delete[] start_;
}

std::streambuf::int_type fdbuf::underflow() {
  if(gptr() >= egptr()) {
    int backed = std::min((std::size_t)(gptr() - eback()), put_back_);
    if(backed > 0) {
      //std::cerr << "backed: " << backed << std::endl;
      memcpy(base_ - backed, gptr() - backed, backed);
    }
    int qty = read(fd_, base_, buff_sz_);
    //std::cerr << "qty: " << qty << std::endl;
    if(qty < 0) {
      //std::cerr << "err: " <<  strerror(errno) << std::endl;
      return traits_type::eof();
    }
    if(qty == 0) {
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
  if(global) {
#if HAVE_GLOBAL_EXTRACT
    return  RE2::GlobalExtract(text, pattern, rewrite, out);
#else
    std::cerr << "Compiled against libre2 which lacks the GlobalExtract patch, don't use -g with -o" << std::endl;
    exit(-9);
#endif
  } else {
    return RE2::Extract(text, pattern, rewrite, out) ? 1 : 0;
  }
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
  std::string *separator;
};

bool match(const re2::StringPiece &text,
           const re2::RE2& pattern,
           bool whole_line) {
  return whole_line ? RE2::FullMatch(text, pattern) :
    RE2::PartialMatch(text, pattern);
}

int str_to_size(const char* str) {
  int v = 0;
  if(str) {
    v = atoi(str);
  }
  if(v < 0) {
    v = 0;
  }
  return v;
}

void emit_line(struct prefix *prefix, char marker, const std::string s,
               const std::string eol, bool flush_after) {
  if(prefix->separator) {
    std::cout << *prefix->separator << eol;
    prefix->separator = NULL;
  }
  if(prefix->fname) {
    std::cout << prefix->fname << marker;
  }
  if(prefix->line_no >= 0) {
    std::cout << prefix->line_no << marker;
  }
  if(prefix->offset >= 0) {
    std::cout << prefix->offset << marker;
  }
  std::cout << s << eol;
  if(flush_after) {
    std::cout.flush();
  }
}

enum input_type {STDIN, CAT, NAME};

fdbuf *ioexec(char* const* arglist, const char* fname, enum input_type util_input, int blksize = 0) {
  int plumb[2];
  pid_t pid;

  if(pipe(&plumb[0])) {
    std::cerr << "error piping for " << arglist[0] << ": "
              << strerror(errno) << std::endl;
    return NULL;
  }
  pid = fork();
  if(pid < 0) {
    std::cerr << "error forking for  " << arglist[0] << ": "
              << strerror(errno) << std::endl;
    return NULL;
  }
  if(!pid) {
    if(util_input != STDIN) {
      close(STDIN_FILENO);
    }

    if(util_input == CAT) {
      int ifd = open(fname, O_RDONLY);
      if(ifd < 0) {
        std::cerr << "error opening " << fname << ": "
                  << strerror(errno) << std::endl;
        std::exit(-1);
      }
      if(dup2(ifd, STDIN_FILENO) != STDIN_FILENO) {
        std::cerr << "error dup2ing for input " << arglist[0] << ": "
                  << strerror(errno) << std::endl;
        std::exit(-2);
      }
    }
    close(STDOUT_FILENO);
    fcntl(STDERR_FILENO, F_SETFD, 1);
    close(plumb[0]);
    if(dup2(plumb[1], STDOUT_FILENO) != STDOUT_FILENO) {
      std::cerr << "error dup2ing for output " << arglist[0] << ": "
                << strerror(errno) << std::endl;
      std::exit(-3);
    }

    execvp(arglist[0], &arglist[0]);
    std::cerr << "error invoking  " << arglist[0] << ": "
              << strerror(errno) << std::endl;
    std::exit(-4);
  }

  if(util_input == STDIN) {
    close(STDIN_FILENO);
  }
  close(plumb[1]);
  if(blksize > 0) {
    return new re2g::fdbuf(plumb[0], blksize);
  } else {
    return new re2g::fdbuf(plumb[0]);
  }
}

};

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
    o_version = 0,
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
    o_line_buffered = isatty(STDOUT_FILENO),
    o_no_group_separator = 0,
    o_group_separator = 0,
    o_set_operator = 0;

  enum {SEARCH, REPLACE} mode = SEARCH;
  enum {MATCH_ANY, MATCH_ALL} multi_rule = MATCH_ANY;

  const struct option options[] = {
    {"help", no_argument, &o_usage, '?'},
    {"global", no_argument, &o_global, 'g'},
    {"invert-match", no_argument, &o_negate_match, 'v'},
    {"only-matching", no_argument, &o_global, 'o'},
    {"subtitute", required_argument, &o_substitute, 's'},
    {"print-all", no_argument, &o_also_print_unreplaced, 'p'},
    {"print-names", no_argument, &o_print_fname, 'H'},
    {"no-print-names", no_argument, &o_no_print_fname, 'h'},
    {"count", no_argument, &o_count, 'c'},
    {"files-with-matches", no_argument, &o_list, 'l'},
    {"files-without-match", no_argument, &o_neg_list, 'L'},
    {"ignore-case", no_argument, &o_case_insensitive, 'i'},
    {"fixed-strings", no_argument, &o_literal, 'F'},
    {"line-regexp", no_argument, &o_full_line, 'x'},
    {"after-context", required_argument, NULL, 'A'},
    {"before-context", required_argument, NULL, 'B'},
    {"context", optional_argument, NULL, 'C'},
    {"line-number", no_argument, NULL, 'n'},
    {"max-count", required_argument, &o_max_matches, 'm'},
    {"exec", required_argument, NULL, 'X'},
    {"quiet", no_argument, &o_quiet_and_quick, 'q'},
    {"silent", no_argument, &o_quiet_and_quick, 'q'},
    {"null", no_argument, &o_special_delimiter, '0'},
    {"line-buffered", no_argument, &o_line_buffered, 'N'},
    {"decompress", no_argument, &o_line_buffered, 'Z'},
    {"gzdecompress", no_argument, &o_line_buffered, 'z'},
    {"bz2decompress", no_argument, &o_line_buffered, 'J'},
    {"extended-regexp", no_argument, &o_posix_extended_syntax, 'E'},
    {"regexp", required_argument, &o_pat_str, 'e'},
    {"file", required_argument, &o_pat_file, 'f'},
    {"no-group-separator", no_argument, &o_no_group_separator, 'T'},
    {"group-separator", required_argument, &o_group_separator, 't'},
    {"version", no_argument, &o_version, 2},
    {"match-any", no_argument, &o_set_operator, 'U'},
    {"match-all", no_argument, &o_set_operator, 'V'},
    { NULL, 0, NULL, 0 }
  };

  std::string rep;
  std::string *group_separator = NULL;
  std::deque<std::string> pat_strs;
  std::deque<std::string> pat_files;
  std::string eol("\n");
  std::deque<std::string> uargs(0);
  char c;
  int longopt = 0;
  int maxopt = sizeof(options) - 1;
  while((c = getopt_long(argc, (char * const *)argv,
                         "?ogvgs:pHhclLiFxB:C:A:nm:X:qN0zZJEe:f:Tt:VU",
                         (const struct option *)&options[0], &longopt)) != -1) {
    if(0 == c && longopt >= 0 &&
       longopt < maxopt) {
      c = options[longopt].val;
    }
    switch(c) {
    case '0':
      o_special_delimiter = 1;
      if(optarg) {
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
      rep = std::string(optarg);
      o_substitute = 1;
      break;
    case 'H':
      o_print_fname = 1;
      break;
    case 'h':
      o_no_print_fname = 1;
      break;
    case 2:
      o_version = 1;
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
      o_after_context = re2g::str_to_size(optarg);
      break;
    case 'B':
      o_before_context = re2g::str_to_size(optarg);
      break;
    case 'C':
      o_after_context = o_before_context = optarg ? re2g::str_to_size(optarg) : 2;
      break;
    case 'm':
      o_max_matches = re2g::str_to_size(optarg);
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
            && !(oa[0] == ';' && oa[1] == 0) ;
          oa = argv[optind++]) {
        uargs.push_back(oa);
      }
      if(!(oa[0] == ';' && oa[1] == 0)) {
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
    case 'V':
      o_set_operator = 1;
      multi_rule = MATCH_ALL;
      break;
    case 'U':
      o_set_operator = 1;
      multi_rule = MATCH_ANY;
      break;
    case 't':
      o_group_separator = 1;
      if(group_separator) {
        *group_separator = optarg;
      } else {
        group_separator = new std::string(optarg);
      }
      break;
    case 'T':
      o_no_group_separator = 1;
      o_group_separator = 0;
      if(group_separator) {
        delete group_separator;
      }
      group_separator = NULL;
      break;
    default:
      o_usage = 1;
    }
  }
  argc -= optind;
  argv += optind;

  if(o_version){
    std::printf(RE2G_VERSION, appname);
    std::printf("\n");
    return 0;
  }

  if(o_neg_list) {
    o_list = 1;
  }
  if(o_negate_match && ! o_set_operator){
    multi_rule = MATCH_ALL;
  }

  /*
  for(const struct option *o=&options[0];o->name;o++) {
    std::cout << o->name << ':';
    if(o->flag) {
      std::cout << *o->flag;
    } else {
      std::cout << "NULL";
    }
    std::cout << std::endl;
  }

  std::cout << "C: [" << o_after_context << ',' << o_before_context << ']' << std::endl;

  std::cout << "ARGC: " << argc << std::endl;
  for(int i=0;i<argc;i++) {
    std::cout << "ARGV[" << i << "]: " << argv[i]  << std::endl;
  }
  std::cout   << std::endl;
  */
  if(argc >= 1 || o_pat_str || o_pat_file) {
    mode = o_substitute ? REPLACE : SEARCH;
  } else {
    o_usage = 1;
  }

  if(o_usage) {
    printf(re2g::USAGE, appname);
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

  while(!pat_files.empty()) {
    std::ifstream patf(pat_files.front().c_str());
    if(!patf) {
      std::cerr << appname << ": " << pat_files.front() << ": " << strerror(errno) << std::endl;
      return -1;
    }
    std::string pat_str;
    while(std::getline(patf, pat_str)) {
      pat_strs.push_back(pat_str);
    }
    pat_files.pop_front();
  }
  if(!o_pat_str && !o_pat_file) {
    pat_strs.push_back(argv[0]);
    argv++;
    argc--;
  }

  while(!pat_strs.empty()) {
    pats.push_back(new RE2::RE2(pat_strs.front(), opts));
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

  if((o_after_context > 0 || o_before_context > 0) &&
     (!o_no_group_separator && !o_group_separator)) {
    group_separator = new std::string("--");
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
  if(!uargs.empty()) {
    uargv = new const char*[uargc + 1];
    uargv[uargc] = NULL;
    for(int aidx = 0; aidx < uargc; aidx++) {
      std::string *arg = &uargs[aidx];
      if(re2g::match(*arg, uarg_pat, false)) {
        uargv[aidx] = NULL;
        rargc++;
      } else {
        uargv[aidx] = arg->c_str();
      }
    }
  }
  std::deque<std::string> rargs(rargc);
  int retval = 1;
  for(int fidx = 0; fidx < num_files; fidx++) {
    const char* fname = fnames[fidx];
    const char* file_err = NULL;
    int blksize = 0;
    if(fname[0] == '-' && fname[1] == 0) {
      fname = def_fname;
      using_stdin = true;
    } else {
      struct stat sout;
      if(stat(fname, &sout) != 0) {
        file_err = strerror(errno);
      } else {
        blksize = sout.st_blksize;
        if(!(sout.st_mode & (S_IFIFO | S_IFREG))) {
          if(sout.st_mode & S_IFDIR) {
            file_err = "Is a directory";
          } else {
            file_err = "Is not a regular file or a pipe";
          }
        }
      }
    }
    if(file_err) {
      std::cerr << appname << ": " << fname << ": " << file_err << std::endl;
    } else {
      std::istream *is = NULL;
      re2g::fdbuf *pb = NULL;
      std::filebuf *fb = NULL;
      if(uargv) {
        int ridx = 0;
        //      std::cout << "fork: ";
        for(int aidx = 0; aidx < uargc; aidx++) {
          if(uargv[aidx] == NULL) {
            rargs[ridx] = std::string(uargs[aidx]);
            re2g::replace(&rargs[ridx], uarg_pat, fname, true);
            uargv[aidx] = rargs[ridx].c_str();
            ridx++;
          }
          //        std::cout << "   " << uargv[aidx] << ',';
        }
        enum re2g::input_type util_input;
        if(rargc) {
          util_input = re2g::NAME;
        } else {
          util_input = using_stdin ? re2g::STDIN : re2g::CAT;
        }

        pb = ioexec((char * const *)uargv, fname, util_input, blksize);
        if(!pb) {
          std::cerr << appname << ": failed to use utility: " << strerror(errno) << std::endl;
          return -1;
        }
        is = new std::istream(pb);
      } else if(using_stdin) {
        is = &std::cin;
      } else {
        fb = new std::filebuf();
        if(fb->open(fname, std::ios_base::in)) {
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
        struct re2g::prefix pref = {
          o_print_fname ? fname : NULL,
          o_print_lineno ? 1 : -1,
          -1,
          NULL
        };
        int line_no = 1;
        int last_line_no = -1;
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
              ++pat) {
            bool this_pat_matched = false;

            if(mode == SEARCH) {
              this_pat_matched = o_negate_match ^ re2g::match(in, **pat, o_full_line);
              to_print = &in;
            } else if(mode == REPLACE) {
              // need to pick: (-o) Extract, (default) Replace, (-g)GlobalReplace
              // also, print non matching lines? (-p)

              if(o_print_match) {
                this_pat_matched = o_negate_match ^ (re2g::extract(in, **pat, rep, &out, o_global) > 0);
                to_print = &out;
              } else {
                this_pat_matched = o_negate_match ^ (re2g::replace(&in, **pat, rep, o_global) > 0);
                to_print = &in;
              }

              if(o_also_print_unreplaced && !this_pat_matched) {
                to_print = &line;
              }
            }
            if(this_pat_matched) {//TODO: check multi-rule
              num_pats_matched++;
              if(mode == REPLACE) {
                if(!obuf.empty()) {
                  obuf += '\n'; //TODO: use configurable line ending
                }
                obuf += *to_print;
              }
            }
          }

          bool right_pats_matched = multi_rule==MATCH_ANY?
            num_pats_matched > 0:
            num_pats_matched == pats.size();

          if(right_pats_matched && mode == REPLACE) {
            to_print = &obuf;
          }
          if(!(right_pats_matched || o_also_print_unreplaced)) {
            to_print = NULL;
          }
          if(right_pats_matched) {
            if(o_quiet_and_quick) {
              return 0;
            }
            count++;
            ca_printed = 0;
            if(o_before_context) {
              line_no -= before.size();
              pref.separator = (count > 1 &&
                                line_no - last_line_no > 1) ? group_separator : NULL;
              while(!before.empty()) {
                pref.line_no = (pref.line_no >= 0) ? line_no : -1;
                re2g::emit_line(&pref, '-', before.front(), eol, o_line_buffered);
                before.pop_front();
                line_no++;
              }
              last_line_no = line_no;
            }
          }
          if(ca_printed < o_after_context && !to_print && !o_count) {
            to_print = &line;
            ca_printed++;
          }
          if(to_print) {
            if(!o_count && !o_list) {
              pref.line_no = (pref.line_no >= 0) ? line_no : -1;
              pref.separator = (count > 1 &&
                                line_no - last_line_no > 1) ? group_separator : NULL;
              re2g::emit_line(&pref, right_pats_matched ? ':' : '-', *to_print, eol, o_line_buffered);
              last_line_no = line_no;
            }
          } else {
            if(o_before_context > 0) {
	      int bs = before.size();
              if(bs >= o_before_context) {
                before.pop_front();
              }
              before.push_back(std::string(line));
            }
          }
          if(line_no >= 0) {
            line_no++;
          }
        }
        if(count > 0) {
          retval = 0;
        }
        if(o_count) {
          if(o_print_fname) {
            std::cout << fname << ":";
          }
          std::cout << count << eol;
          if(o_line_buffered) {
            std::cout.flush();
          }
        } else if(o_list && (count > 0) ^ o_neg_list) {
          std::cout << fname << eol;
          if(o_line_buffered) {
            std::cout.flush();
          }
        }
        if(using_stdin) {
          using_stdin = false;
        } else {
          delete is;
        }
      }
      if(pb) {
        int sl;
        wait(&sl);
        delete pb;
      }
      if(fb) {
        delete fb;
      }
    }
  }
  if(group_separator) {
    delete group_separator;
  }
  while(!pats.empty()) {
    delete pats.back();
    pats.pop_back();
  }

  if(uargv) {
    delete[] uargv;
  }
  return retval;
}
