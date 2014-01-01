// {petry,rahn}@itwm.fhg.de

#ifndef WE_MGMT_TYPE_ACTIVITY_HPP
#define WE_MGMT_TYPE_ACTIVITY_HPP 1

#include <we/mgmt/type/activity.fwd.hpp>

#include <we/type/id.hpp>
#include <we/type/transition.hpp>

#include <we/mgmt/context.fwd.hpp>

#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <boost/filesystem.hpp>
#include <boost/random.hpp>

#include <vector>

#include <iosfwd>

namespace we
{
  namespace mgmt
  {
    namespace type
    {
      class activity_t
      {
      public:
        typedef std::pair< pnet::type::value::value_type
                         , petri_net::port_id_type
                         > token_on_port_t;
        typedef std::vector<token_on_port_t> token_on_port_list_t;
        typedef token_on_port_list_t input_t;
        typedef token_on_port_list_t output_t;

      private:
        typedef boost::unique_lock<boost::recursive_mutex> shared_lock_t;
        typedef boost::unique_lock<boost::recursive_mutex> unique_lock_t;

      public:
        explicit activity_t ();
        explicit activity_t (const we::type::transition_t&);
        activity_t (const activity_t&);
        explicit activity_t (const boost::filesystem::path&);
        explicit activity_t (std::istream&);
        explicit activity_t (const std::string&);

        activity_t& operator= (const activity_t&);

        std::string to_string() const;

        //! \todo DIRTY! Why lock and return a ref? Eliminate!!
        const we::type::transition_t& transition() const;
        we::type::transition_t& transition();

        activity_t extract();
        void inject (const activity_t&);
        void collect_output();

        int execute (context*);

        bool can_fire() const;

        const input_t& input() const;
        void add_input
          ( petri_net::port_id_type const&
          , pnet::type::value::value_type const&
          );

        const output_t& output() const;
        void set_output (const output_t&);
        void add_output (const output_t::value_type&);

        std::string nice_name() const;

      private:
        template<class Archive>
          void save (Archive& ar, const token_on_port_list_t& l) const
        {
          const std::size_t size (l.size());
          ar & size;
          BOOST_FOREACH (const token_on_port_t& top, l)
          {
            std::ostringstream oss;
            oss << pnet::type::value::show (top.first);
            const std::string rep (oss.str());
            ar & rep;
            ar & top.second;
          }
        }
        template<class Archive>
          void load (Archive& ar, token_on_port_list_t& l) const
        {
          std::size_t size;
          ar & size;
          while (size --> 0)
          {
            std::string rep;
            ar & rep;
            petri_net::port_id_type port_id;
            ar & port_id;
            l.push_back (std::make_pair ( pnet::type::value::read (rep)
                                        , port_id
                                        )
                        );
          }
        }

        friend class boost::serialization::access;
        template<class Archive>
          void save (Archive& ar, const unsigned int) const
        {
          unique_lock_t lock (_mutex);

          ar & BOOST_SERIALIZATION_NVP(_transition);

          save (ar, _input);
          save (ar, _output);
        }
        template<class Archive>
        void load (Archive& ar, const unsigned int)
        {
          unique_lock_t lock (_mutex);

          ar & BOOST_SERIALIZATION_NVP(_transition);

          load (ar, _input);
          load (ar, _output);
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

      private:
        mutable boost::recursive_mutex _mutex;

        we::type::transition_t _transition;

        input_t _input;
        output_t _output;

        boost::mt19937 _engine;
      };

      std::ostream& operator<< (std::ostream&, const activity_t&);
    }
  }
}

#endif
