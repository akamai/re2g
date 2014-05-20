#include <streambuf>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <errno.h>


#include <iostream>
#include <string>

class pbuff : public std::streambuf {
public:
  explicit pbuff(int fd, std::size_t buff_sz = 256, std::size_t put_back = 8);
  ~pbuff();
  
private:
  // overrides base class underflow()
  std::streambuf::int_type underflow();
  
  // copy ctor and assignment not implemented;
  // copying not allowed
  pbuff(const pbuff &);
  pbuff &operator= (const pbuff &);
  
private:
  int fd_;
  const std::size_t put_back_;
  char* base_;
  char* start_;
  char* end_;
  int full_sz_;
  int buff_sz_;
};

pbuff::pbuff(int fd, std::size_t buff_sz, std::size_t put_back) :
    fd_(fd),
    put_back_(std::max(put_back, size_t(1))),
    buff_sz_(std::max(buff_sz, put_back_)),
    full_sz_(buff_sz_ + put_back_),
    start_(new char[full_sz_])
{
  base_ = start_ + put_back_;
  end_ = start_ + full_sz_;

  setg(end_,end_,end_);

}

pbuff::~pbuff() {
  delete[] start_;
}

std::streambuf::int_type pbuff::underflow(){
  if(gptr() >= egptr()){
    int backed = std::min((std::size_t)(gptr() - eback()), put_back_);
    if(backed > 0){
      //std::cerr << "backed: " << backed << std::endl;
      std::memcpy(base_ - backed, gptr()-backed, backed);
    }
    int qty = read(fd_, base_, buff_sz_);
    //std::cerr << "qty: " << qty << std::endl;
    if(qty < 0){
      std::cerr << "err: " <<  strerror(errno) << std::endl;
    }
    if(qty == 0){
      //std::cerr << "EOF" <<std::endl;
      return traits_type::eof();
    }
    setg(start_, base_, base_ + qty);
  }
  return traits_type::to_int_type(*gptr());
}


int main (int argc, char** argv){
  pbuff pb(0,10,5);
  std::string line;
  std::istream is(&pb);
  while(std::getline(is, line)){
    std::cout << "read: " << line << std::endl;
  }
  return 0;
}
