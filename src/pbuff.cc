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
  std::vector<char> buffer_;
};

pbuff::pbuff(int fd, std::size_t buff_sz, std::size_t put_back) :
    fd_(fd),
    put_back_(std::max(put_back, size_t(1))),
    buffer_(std::max(buff_sz, put_back_) + put_back_)
{
  setg(&buffer_.back()+1,&buffer_.back()+1,&buffer_.back()+1);
}

std::streambuf::int_type pbuff::underflow(){
  if(gptr() >= egptr()){
    char* base = &buffer_[put_back_];
    int backed = std::min((std::size_t)(gptr() - eback()), put_back_);
    if(backed > 0){
      //std::cerr << "backed: " << backed << std::endl;
      std::memcpy(base - backed, gptr()-backed, backed);
    }
    int qty = read(fd_, base, buffer_.size()-put_back_);
    //std::cerr << "qty: " << qty << std::endl;
    if(qty < 0){
      std::cerr << "err: " <<  strerror(errno) << std::endl;
    }
    if(qty == 0){
      //std::cerr << "EOF" <<std::endl;
      return traits_type::eof();
    }
    setg(&buffer_.front(), base, base + qty);
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
