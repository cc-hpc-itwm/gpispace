#include "message.hpp"

#include <sstream>
#include <iomanip>

using namespace fhg::com;

message::message ()
  : data_()
  , header_(encode_header())
{}

message::message (std::string const & _data)
  : data_(_data.c_str(), _data.c_str()+_data.size())
  , header_(encode_header())
{}

std::string message::encode_header () const
{
  std::ostringstream sstr;
  sstr << std::setw(header_length)
       << std::setfill('0')
       << std::hex
       << data_.size()
    ;
  return sstr.str();
}

void message::resize(const std::size_t n)
{
  data_.resize (n);
}

const std::vector<char> & message::data (void) const
{
  return data_;
}

const std::string & message::header (void) const
{
  return header_;
}
