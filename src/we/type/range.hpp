// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_RANGE_HPP
#define WE_TYPE_RANGE_HPP

#include <we/field.hpp>
#include <we/type/value.hpp>

#include <fhg/assert.hpp>

#include <string>

namespace we
{
  struct interval
  {
  public:
    interval (pnet::type::value::value_type const& value)
      : _offset ( pnet::field_as<unsigned long>
                  ("offset", value, std::string ("unsigned long"))
                )
      , _size ( pnet::field_as<unsigned long>
                ("size", value, std::string ("unsigned long"))
              )
    {}
    interval (interval const& r, unsigned long size)
      : _offset (r.offset())
      , _size (size)
    {
      fhg_assert (size <= r.size());
    }
    void shrink (unsigned long delta)
    {
      fhg_assert (delta <= _size);

      _offset += delta;
      _size -= delta;
    }
    unsigned long offset() const
    {
      return _offset;
    }
    unsigned long size() const
    {
      return _size;
    }

  private:
    unsigned long _offset;
    unsigned long _size;
  };

  namespace global
  {
    struct handle
    {
    public:
      handle (pnet::type::value::value_type const& value)
        : _name ( pnet::field_as<std::string>
                  ("name", value, std::string ("string"))
                )
      {}
      std::string name() const
      {
        return _name;
      }
    private:
      std::string _name;
    };

    struct range : public interval
    {
    public:
      range (pnet::type::value::value_type const& value)
        : interval (value)
        , _handle (pnet::field ("handle", value, std::string ("handle")))
      {}
      range (range const& r, unsigned long size)
        : interval (r, size)
        , _handle (r.handle())
      {}
      struct handle const& handle() const
      {
        return _handle;
      }

    private:
      struct handle _handle;
    };
  }

  namespace local
  {
    struct range : interval
    {
    public:
      range (pnet::type::value::value_type const& value)
        : interval (value)
        , _buffer ( pnet::field_as<std::string>
                    ("buffer", value, std::string ("string"))
                  )
      {}
      range (range const& r, unsigned long size)
        : interval (r, size)
        , _buffer (r.buffer())
      {}
      std::string const& buffer() const
      {
        return _buffer;
      }
    private:
      std::string _buffer;
    };
  }
}

#endif
