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
        explicit From(const address_type &val)
          : val(val) {}
        address_type val;
    };
    class To {
      public:
        explicit To(const address_type &val)
          : val(val) {}
        address_type val;
    };

    class SedaMessage : public seda::UserEvent, public seda::comm::Encodeable, public seda::comm::Decodeable {
      public:
        typedef std::tr1::shared_ptr<SedaMessage> Ptr;

        explicit
          SedaMessage()
          : from_(address_type()), to_(address_type()), payload_(payload_type()), valid_(false) {}
        SedaMessage(const address_type & from, const address_type & to, const payload_type & payload)
          : from_(from), to_(to), payload_(payload), valid_(true) { }

        SedaMessage(const SedaMessage &other)
          : from_(other.from()), to_(other.to()), payload_(other.payload()), valid_(other.is_valid()) {}
        virtual ~SedaMessage() {}

        const SedaMessage &operator=(const SedaMessage &rhs) {
          if (this != &rhs) {
            from_ = rhs.from();
            to_ = rhs.to();
            payload_ = rhs.payload();
            valid_ = rhs.is_valid();
            encode_buf_ = rhs.encode_buf_;
            strrep_buf_ = rhs.strrep_buf_;
          }
          return *this;
        }

        std::string str() const;
        virtual void decode(const std::string&) throw(DecodingError);
        virtual const std::string &encode() const throw(EncodingError);

        const address_type & from() const { return from_; }
        const address_type & to() const { return to_; }
        const payload_type & payload() const { return payload_; }
        bool is_valid() const { return valid_; }

        void from(const address_type &new_from) { from_ = new_from; reset_buffers(); }
        void to(const address_type &new_to) { to_ = new_to; reset_buffers(); }
        void payload(const payload_type &new_payload) { payload_ = new_payload; reset_buffers(); }
      private:
        void reset_buffers()
        {
          encode_buf_.clear();
          strrep_buf_.clear();
        }

        address_type from_;
        address_type to_;
        payload_type payload_;
        bool valid_;
        mutable std::string encode_buf_;
        mutable std::string strrep_buf_;
    };    
  }}

#endif
