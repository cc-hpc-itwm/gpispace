#pragma once

#include <fhglog/Logger.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/memory_location.hpp>

#include <we/type/range.hpp>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <list>
#include <map>
#include <string>
#include <utility>

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      class api_t : public boost::noncopyable
      {
      public:
        typedef boost::shared_ptr<gpi::pc::segment::segment_t> segment_ptr;
        typedef std::map<gpi::pc::type::segment_id_t, segment_ptr> segment_map_t;

        api_t (fhg::log::Logger&, std::string const & path);

        ~api_t ();

        //! \todo add const (do not call stop()) in memcpy, ptr, wait
        //! and replace /*const*/ in
        //!   src/drts/worker/drts-kernel.cpp
        //!   src/drts/worker/drts.cpp
        //!   src/drts/worker/drts.hpp
        //!   src/we/loader/module_call.cpp
        //!   src/we/loader/module_call.hpp

        // api to gpi
        type::handle_id_t alloc ( const type::segment_id_t
                                , const type::size_t
                                , const std::string & desc
                                , const type::flags_t
                                );
        void free (const type::handle_id_t);

        gpi::pc::type::queue_id_t
        memcpy ( gpi::pc::type::memory_location_t const & dst
               , gpi::pc::type::memory_location_t const & src
               , const gpi::pc::type::size_t amount
               );

        std::function<double (std::string const&)>
        transfer_costs (std::list<std::pair<we::local::range, we::global::range>> const&);

        std::map<std::string, double>
        transfer_costs (std::list<gpi::pc::type::memory_region_t> const&);

        void * ptr(const gpi::pc::type::handle_t h);

        gpi::pc::type::size_t
        wait (const gpi::pc::type::queue_id_t);

        gpi::pc::type::segment_id_t register_segment( std::string const & name
                                                    , const gpi::pc::type::size_t sz
                                                    , const gpi::pc::type::flags_t
                                                    );
        void unregister_segment(const gpi::pc::type::segment_id_t);

      private:
        void stop ();
        gpi::pc::type::handle::descriptor_t
        info(const gpi::pc::type::handle_t h);


        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        gpi::pc::proto::message_t communicate (gpi::pc::proto::message_t const &);

        fhg::log::Logger& _logger;
        mutable mutex_type m_mutex;
        int m_socket;
        segment_map_t m_segments;
      };
    }
  }
}
