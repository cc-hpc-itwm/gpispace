#ifndef FHG_COM_MESSAGE_HPP
#define FHG_COM_MESSAGE_HPP 1

#include <vector>
#include <string>

namespace fhg
{
  namespace com
  {
    class message
    {
    public:
#define FHG_COM_MESSAGE_LENGTH_TYPE std::size_t

      // header length is encoded as hex chars (2 chars per byte)
      enum { header_length = sizeof(FHG_COM_MESSAGE_LENGTH_TYPE) * 2 };

      message ();
      message (std::string const & _data);

      const std::vector<char> & data () const;
      const std::string & header () const;

      std::string encode_header (void) const;
      void resize (const std::size_t);

    private:
      std::vector<char> data_;
      std::string header_;
    };
  }
}

#endif
