#include "SedaMessage.hpp"
#include "seda-msg.pb.h"

#include <sstream>

using namespace seda::comm;

void SedaMessage::decode(const std::string &s) throw(DecodingError) {
  Message m;
  m.ParseFromString(s);
  if (! m.has_to()) throw DecodingError("the message did not contain the required field 'to'");
  if (! m.has_from()) throw DecodingError("the message did not contain the required field 'from'");
  if (! m.has_payload()) throw DecodingError("the message did not contain the required field 'payload'");

  to_ = m.to();
  from_ = m.from();
  payload_ = m.payload();
  type_code_ = m.type();
  encode_buf_.clear();
  strrep_buf_.clear();
}

const std::string &SedaMessage::encode() const throw(EncodingError) {
  if (encode_buf_.empty())
  {
    Message m;
    m.set_to(to());
    m.set_from(from());
    m.set_payload(payload());
    m.set_type(type_code());

    m.SerializeToString(&encode_buf_);
  }
  return encode_buf_;
}

std::string SedaMessage::str() const {
  if (strrep_buf_.empty())
  {
    std::ostringstream os;
    os << "SedaMessage (type=" << type_code() << "): " << from() << " --> " << to() << ": '" << payload() << "'";
    strrep_buf_ = os.str();
  }
  return strrep_buf_;
}

