// alexander.petry@itwm.fraunhofer.de

#ifndef DRTS_STREAM_HPP
#define DRTS_STREAM_HPP

#include <drts/drts.fwd.hpp>
#include <drts/virtual_memory.fwd.hpp>

#include <we/type/value.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace gspc
{
  class stream
  {
  public:
    struct size_of_slot
    {
      explicit size_of_slot (std::size_t size)
        : _ (size)
        {}

      operator std::size_t const& () const
      {
        return _;
      }
    private:
      const std::size_t _;
    };
    struct number_of_slots
    {
      explicit number_of_slots (std::size_t count)
        : _ (count)
      {}

      operator std::size_t const& () const
      {
        return _;
      }
    private:
      const std::size_t _;
    };

  private:
    friend class scoped_runtime_system;

    stream ( scoped_runtime_system const&
           , std::string const& name
           , gspc::vmem_allocation const& buffer
           , gspc::vmem_allocation const& meta
           , size_of_slot const&
           , number_of_slots const&
           , std::function<void (pnet::type::value::value_type const&)> on_slot_filled
           );
  public:
    ~stream();

    void write (std::string const&);

    static void mark_free ( const char old_flag_value
                          , std::pair<void*, unsigned long> ptr_to_flag
                          );
    static std::size_t size_of_meta_data_slot();

    stream (stream const&) = delete;
    stream& operator= (stream const&) = delete;

    stream (stream&&);
    stream& operator= (stream&&) = delete;
  private:
    struct implementation;
    std::unique_ptr<implementation> _;
  };
}

#endif
