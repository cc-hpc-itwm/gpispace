#include <gspc/stencil_cache/callback.hpp>

#include <we/type/stencil_cache.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/this_bound_mem_fn.hpp>

#include <boost/format.hpp>

#include <algorithm>
#include <list>
#include <stdexcept>
#include <utility>

namespace we
{
  namespace type
  {
    namespace
    {
      pnet::type::value::value_type const& peek
        ( std::string key
        , pnet::type::value::value_type const& x
        )
      {
        auto const value (pnet::type::value::peek ({key}, x));

        if (!value)
        {
          throw std::logic_error
            ( ( boost::format ("Missing binding for '%1%' in '%2%'.")
              % key
              % pnet::type::value::show (x)
              ).str()
            );
        }

        return value.get();
      }

      stencil_cache::SCache::InputEntries input_entries
        ( expr::eval::context const& context
        , scoped_neighbors_callback& _neighbors
        , stencil_cache::Slot M
        )
      {
        auto const begin
          ( boost::get<gspc::stencil_cache::Coordinate>
              (peek ("value", context.value ({"begin"})))
          );
        auto const end
          ( boost::get<gspc::stencil_cache::Coordinate>
              (peek ("value", context.value ({"end"})))
          );

        stencil_cache::SCache::InputEntries input_entries;

        std::size_t max {0};

        for (gspc::stencil_cache::Coordinate c {begin}; c < end; ++c)
        {
          std::list<gspc::stencil_cache::Coordinate> const ns (_neighbors (c));

          max = std::max (max, ns.size());

          for (auto const neighbor : ns)
          {
            input_entries[neighbor].increment();
          }
        }

        if (M < max)
        {
          throw std::invalid_argument
            ( ( boost::format ("stencil_cache: Not enough memory (%1% < %2%)")
              % M
              % max
              ).str()
            );
        }

        return input_entries;
      }

      std::string place ( expr::eval::context const& context
                        , std::string key
                        )
      {
        return boost::get<std::string> (context.value ({"place", key}));
      }
    }

    scoped_neighbors_callback::scoped_neighbors_callback
      ( std::string implementation
      , std::vector<char> const& data
      )
        : _implementation (implementation)
        , _state ( FHG_UTIL_SCOPED_DLHANDLE_SYMBOL
                   (_implementation, gspc_stencil_cache_callback_init) (data)
                 )
        , _neighbors ( FHG_UTIL_SCOPED_DLHANDLE_SYMBOL
                       (_implementation, gspc_stencil_cache_callback_neighbors)
                     )
    {}

    std::list<scoped_neighbors_callback::Coordinate>
      scoped_neighbors_callback::operator() (Coordinate c) const
    {
      return _neighbors (_state.get(), c);
    }

    pnet::type::value::value_type
      stencil_cache::global_range (stencil_cache::Slot slot) const
    {
      using pnet::type::value::value_type;
      using pnet::type::value::poke;
      using Path = std::list<std::string>;

      value_type range;
      poke (Path {"handle"}, range, _handle);
      poke (Path {"offset"}, range, _base + slot * _block_size);
      poke (Path {"size"}, range, _input_size);

      return range;
    }

    stencil_cache::stencil_cache ( expr::eval::context const& context
                                 , PutToken const& put_token
                                 )
      : _place_prepare (place (context, "prepare"))
      , _place_ready (place (context, "ready"))
      , _place_neighbors (place (context, "neighbors"))
      , _place_neighbors_count (place (context, "neighbors_count"))
      , _memory (context.value ({"memory"}))
      , _handle (peek ("handle", _memory))
      , _base (boost::get<unsigned long> (peek ("offset", _memory)))
      , _input_size (boost::get<unsigned long> (context.value ({"input_size"})))
      , _block_size (boost::get<unsigned long> (context.value ({"block_size"})))
      , _M (boost::get<unsigned long> (peek ("size", _memory)) / _block_size)
      , _put_token (put_token)
      , _neighbors
        ( boost::get<std::string> (peek ("path", context.value ({"neighbors"})))
        , boost::get<we::type::bytearray>
            (peek ("data", context.value ({"neighbors"}))).v()
        )
      , _scache ( input_entries (context, _neighbors, _M)
                , fhg::util::bind_this (this, &stencil_cache::prepare)
                , fhg::util::bind_this (this, &stencil_cache::ready)
                , _M
                )
      , _allocate (fhg::util::bind_this (this, &stencil_cache::allocate))
    {}

    stencil_cache::~stencil_cache()
    {
      _queue_allocate.interrupt();
      _scache.interrupt();
      _allocate.join();
    }

    void stencil_cache::allocate()
    try
    {
      while (true)
      {
        _scache.alloc (_queue_allocate.get());
      }
    }
    catch (QAllocate::interrupted)
    {
      return;
    }
    catch (SCache::interrupted)
    {
      return;
    }

    void stencil_cache::alloc (expr::eval::context const& context)
    {
      auto const stencil
        ( boost::get<gspc::stencil_cache::Stencil>
            (peek ("value", context.value ({"stencil"})))
        );

      auto const neighbors (_neighbors (stencil));

      _queue_allocate.put (Allocate {stencil, neighbors});

      for (auto const& neighbor : neighbors)
      {
        using pnet::type::value::value_type;
        using pnet::type::value::poke;
        using Path = std::list<std::string>;

        value_type v;
        poke (Path {"value"}, v, neighbor);

        _put_token (_place_neighbors, std::move (v));
      }

      _put_token (_place_neighbors_count, neighbors.size());
    }

    void stencil_cache::prepare
      (Slot slot, gspc::stencil_cache::Coordinate coordinate) const
    {
      using pnet::type::value::value_type;
      using pnet::type::value::poke;
      using Path = std::list<std::string>;

      std::list<value_type> puts {global_range (slot)};

      value_type v;
      poke (Path {"coordinate", "value"}, v, coordinate);
      poke (Path {"memory_put"}, v, std::move (puts));

      _put_token (_place_prepare, std::move (v));
    }

    void stencil_cache::prepared (expr::eval::context const& context)
    {
      auto const coordinate
        ( boost::get<gspc::stencil_cache::Coordinate>
            (peek ("value", context.value ({"prepared"})))
        );

      _scache.prepared (coordinate);
    }

    void stencil_cache::ready ( gspc::stencil_cache::Stencil stencil
                              , SCache::Assignment assignment
                              ) const
    {
      using pnet::type::value::value_type;
      using pnet::type::value::poke;
      using Path = std::list<std::string>;

      std::list<value_type> wrapped;
      std::list<value_type> gets;

      for (auto const& assigned : assignment)
      {
        value_type v;
        poke (Path {"coordinate"}, v, assigned.second);
        wrapped.emplace_back (std::move (v));

        gets.emplace_back (global_range (assigned.first));
      }

      value_type v;
      poke (Path {"stencil", "value"}, v, stencil);
      poke (Path {"assignment", "assigned"}, v, std::move (wrapped));
      poke (Path {"memory_gets"}, v, std::move (gets));

      _put_token (_place_ready, std::move (v));
    }

    void stencil_cache::free (expr::eval::context const& context)
    {
      auto const useds
        ( boost::get<std::list<pnet::type::value::value_type>>
            (peek ("assigned", context.value ({"assignment"})))
        );

      for (auto const& used : useds)
      {
        auto const coordinate
          ( boost::get<gspc::stencil_cache::Coordinate>
              (peek ("coordinate", used))
          );

        _scache.free (coordinate);
      }
    }
  }
}
