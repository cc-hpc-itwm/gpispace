#include <gspc/stencil_cache/types.hpp>
#include <gspc/stencil_cache/callback.hpp>

#include <we/type/stencil_caches.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/show.hpp>

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
        , fhg::util::scoped_dlhandle& _neighbors_implementation
        , gspc::stencil_cache::Slot M
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

        auto neighbors
          ( FHG_UTIL_SCOPED_DLHANDLE_SYMBOL
              (_neighbors_implementation, gspc_stencil_cache_callback_neighbors)
          );

        stencil_cache::SCache::InputEntries input_entries;

        std::size_t max {0};

        for (gspc::stencil_cache::Coordinate c {begin}; c < end; ++c)
        {
          std::list<gspc::stencil_cache::Coordinate> ns; neighbors (c, ns);

          max = std::max (max, ns.size());

          for (auto const neighbor : ns)
          {
            input_entries[neighbor].increment();
          }
        }

        if (M < max)
        {
          throw std::invalid_argument
            ( ( boost::format ("stencil_caches: Not enough memory (%1% < %2%)")
              % M
              % max
              ).str()
            );
        }

        return input_entries;
      }
    }

    stencil_cache::stencil_cache ( expr::eval::context const& context
                                 , PutToken put_token
                                 )
      : _place_prepare
        (boost::get<std::string> (context.value ({"place", "prepare"})))
      , _place_ready
        (boost::get<std::string> (context.value ({"place", "ready"})))
      , _input_memory (context.value ({"memory"}))
      , _input_size (boost::get<unsigned long> (context.value ({"size"})))
      , _M
        (boost::get<unsigned long> (peek ("size", _input_memory)) / _input_size)
      , _put_token (std::move (put_token))
      , _neighbors_implementation
        (boost::get<std::string> (peek ("path", context.value ({"neighbors"}))))
      , _scache ( input_entries (context, _neighbors_implementation, _M)
                , [this]
                    ( gspc::stencil_cache::Slot slot
                    , gspc::stencil_cache::Coordinate coordinate
                    )
                  {
                    using pnet::type::value::value_type;
                    using pnet::type::value::poke;
                    using Path = std::list<std::string>;

                    value_type v;
                    poke (Path {"slot", "value"}, v, slot);
                    poke (Path {"coordinate", "value"}, v, coordinate);

                    _put_token (_place_prepare, std::move (v));
                  }
                , [this]
                    ( gspc::stencil_cache::Stencil stencil
                    , SCache::Assignment assignment
                    )
                  {
                    using pnet::type::value::value_type;
                    using pnet::type::value::poke;
                    using Path = std::list<std::string>;

                    std::list<value_type> wrapped;
                    std::list<value_type> gets;

                    auto const handle (peek ("handle", _input_memory));
                    auto const base
                      ( boost::get<unsigned long>
                          (peek ("offset", _input_memory))
                      );

                    for (auto const& assigned : assignment)
                    {
                      value_type v;
                      poke (Path {"slot"}, v, assigned.first);
                      poke (Path {"coordinate"}, v, assigned.second);
                      wrapped.emplace_back (std::move (v));

                      value_type g;
                      poke (Path {"handle"}, g, handle);
                      poke (Path {"offset"}, g, base + assigned.first * _input_size);
                      poke (Path {"size"}, g, _input_size);
                      gets.emplace_back (std::move (g));
                    }

                    value_type v;
                    poke (Path {"stencil", "value"}, v, stencil);
                    poke (Path {"assignment", "assigned"}, v, std::move (wrapped));
                    poke (Path {"memory_gets"}, v, std::move (gets));

                    _put_token (_place_ready, std::move (v));
                  }
                , _M
                )
      , _allocate ( [this] ()
                    {
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
                    }
                  )
    {}

    stencil_cache::~stencil_cache()
    {
      _queue_allocate.interrupt();
      _allocate.join();
      _scache.interrupt();
    }

    void stencil_cache::alloc (expr::eval::context const& context)
    {
      auto const stencil
        ( boost::get<gspc::stencil_cache::Stencil>
            (peek ("value", context.value ({"stencil"})))
        );

      auto neighbors
        ( FHG_UTIL_SCOPED_DLHANDLE_SYMBOL
            (_neighbors_implementation, gspc_stencil_cache_callback_neighbors)
        );

      std::list<gspc::stencil_cache::Coordinate> inputs;
      neighbors (stencil, inputs);

      _queue_allocate.put (Allocate {stencil, std::move (inputs)});
    }

    void stencil_cache::prepared (expr::eval::context const& context)
    {
      auto const coordinate
        ( boost::get<gspc::stencil_cache::Coordinate>
            (peek ("value", context.value ({"prepared"})))
        );

      _scache.prepared (coordinate);
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

    void stencil_caches::operator() ( expr::eval::context const& context
                                    , stencil_cache::PutToken put_token
                                    )
    {
      auto const operation
        (boost::get<std::string> (context.value ({"operation"})));

      if ("INIT" == operation)
      {
        auto const id (boost::get<unsigned long> (context.value ({"id"})));

        if ( ! _.emplace
               ( std::piecewise_construct
               , std::forward_as_tuple (id)
               , std::forward_as_tuple (context, std::move (put_token))
               ).second
           )
        {
          throw std::invalid_argument
            ((boost::format ("stencil_caches: Duplicate id %1%") % id).str());
        }

        return;
      }

      auto const id
        (boost::get<unsigned long> (peek ("id", context.value ({"cache"}))));

      auto cache (_.find (id));

      if (cache == _.end())
      {
        throw std::invalid_argument
          ((boost::format ("Unknown cache %1% (%2%)") % id % operation).str());
      }

      if ("ALLOC" == operation)
      {
        cache->second.alloc (context);
      }
      else if ("PREPARED" == operation)
      {
        cache->second.prepared (context);
      }
      else if ("FREE" == operation)
      {
        cache->second.free (context);
      }
      else if ("DESTROY" == operation)
      {
        _.erase (cache);
      }
      else
      {
        throw std::invalid_argument
          ( ( boost::format ("stencil_caches: Unknown operation %1%")
            % operation
            ).str()
          );
      }
    }
  }
}
