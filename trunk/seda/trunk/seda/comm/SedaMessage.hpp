#ifndef SEDA_COMM_MESSAGE_HPP
#define SEDA_COMM_MESSAGE_HPP 1

#include <seda/UserEvent.hpp>
#include <seda/comm/Decodeable.hpp>
#include <seda/comm/Encodeable.hpp>

#include <string>

namespace seda {
  namespace comm {
    typedef std::string payload_type;
    typedef std::string address_type;

    class From {
      public:
        explicit From(const address_type &value)
          : val(value) {}
        address_type val;
    };
    class To {
      public:
        explicit To(const address_type &value)
          : val(value) {}
        address_type val;
    };

    class SedaMessage : public seda::UserEvent, public seda::comm::Encodeable, public seda::comm::Decodeable {
      public:
        typedef std::tr1::shared_ptr<SedaMessage> Ptr;

        explicit
          SedaMessage()
          : from_(address_type())
          , to_(address_type())
          , payload_(payload_type())
          , type_code_(0)
        {}
        SedaMessage(const address_type & a_from, const address_type & a_to, const payload_type & a_payload, unsigned int a_type_code = 0)
          : from_(a_from)
          , to_(a_to)
          , payload_(a_payload)
          , type_code_(a_type_code)
        {}

        SedaMessage(const SedaMessage &other)
          : UserEvent()
          , Encodeable()
          , Decodeable()
          , from_(other.from())
          , to_(other.to())
          , payload_(other.payload())
          , type_code_(other.type_code())
        {}

        virtual ~SedaMessage() {}

        SedaMessage &operator=(const SedaMessage &rhs) {
          if (this != &rhs) {
            from_ = rhs.from();
            to_ = rhs.to();
            payload_ = rhs.payload();
            type_code_ = rhs.type_code();
            encode_buf_ = rhs.encode_buf_;
            strrep_buf_ = rhs.strrep_buf_;
          }
          return *this;
        }

        std::string str() const;
        virtual void decode(const std::string&) throw(DecodingError);
        virtual const std::string &encode() const throw(EncodingError);

        const address_type & from() const { return from_; }
        address_type & from() { return from_; }
        const address_type & to() const { return to_; }
        address_type & to() { return to_; }
        const payload_type & payload() const { return payload_; }
        payload_type & payload() { return payload_; }
        const unsigned int & type_code() const { return type_code_; }
        unsigned int & type_code() { return type_code_; }

        void from(const address_type &new_from) { from_ = new_from; reset_buffers(); }
        void to(const address_type &new_to) { to_ = new_to; reset_buffers(); }
        void payload(const payload_type &new_payload) { payload_ = new_payload; reset_buffers(); }
        void type_code(const unsigned int &new_type_code) { type_code_ = new_type_code; reset_buffers(); }
      private:
        void reset_buffers()
        {
          encode_buf_.clear();
          strrep_buf_.clear();
        }

        address_type from_;
        address_type to_;
        payload_type payload_;
        unsigned int type_code_;
        mutable std::string encode_buf_;
        mutable std::string strrep_buf_;
    };    
  }}

#endif
