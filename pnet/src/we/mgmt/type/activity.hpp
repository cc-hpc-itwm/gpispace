// {petry,rahn}@itwm.fhg.de

#ifndef WE_MGMT_TYPE_ACTIVITY_HPP
#define WE_MGMT_TYPE_ACTIVITY_HPP 1

#include <we/mgmt/type/activity.fwd.hpp>

#include <we/type/id.hpp>
#include <we/type/transition.hpp>
#include <we/expr/eval/context.hpp>

#include <we/mgmt/type/flags.hpp>
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

        template<typename T>
          boost::optional<T> get_schedule_data (const std::string& key) const
        {
          we::type::property::path_type path;
          path.push_back ("fhg");
          path.push_back ("drts");
          path.push_back ("schedule");
          path.push_back (key);

          boost::optional<const we::type::property::value_type&> expr
            (_transition.prop().get_maybe_val (path));

          if (!expr)
          {
            return boost::none;
          }

          we::type::expression_t e (*expr);

          expr::eval::context context;

          for ( input_t::const_iterator top (input().begin())
              ; top != input().end()
              ; ++top
              )
          {
            context.bind_ref ( _transition.get_port (top->second).name()
                             , top->first
                             );
          }

          return boost::get<T> (e.ast().eval_all (context));
        }

        void set_id (const petri_net::activity_id_type&);
        const petri_net::activity_id_type& id() const;
        const flags::flags_t& flags() const;

        bool is_alive() const;

#define FLAG(_name)                             \
        bool is_ ## _name() const;              \
        void set_ ## _name (bool value = true)

        FLAG (suspended);
        FLAG (canceling);
        FLAG (cancelled);
        FLAG (failed);
        FLAG (finished);
#undef FLAG

        //! \todo DIRTY! Why lock and return a ref? Eliminate!!
        const we::type::transition_t& transition() const;
        we::type::transition_t& transition();

        std::string type_to_string() const;

        activity_t extract();
        void inject (const activity_t&);
        void inject_input();
        void collect_output();

        int execute (context*);

        bool can_fire() const;

        const input_t& pending_input() const;
        const input_t& input() const;
        void add_input (const input_t::value_type&);

        const output_t& output() const;
        void set_output (const output_t&);
        void add_output (const output_t::value_type&);

        std::ostream& print (std::ostream&, const token_on_port_list_t&) const;

        std::string nice_name() const;

      private:
        void lock();
        void unlock();

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

          ar & BOOST_SERIALIZATION_NVP(_id);
          ar & BOOST_SERIALIZATION_NVP(_flags);
          ar & BOOST_SERIALIZATION_NVP(_transition);

          save (ar, _pending_input);
          save (ar, _input);
          save (ar, _output);
        }
        template<class Archive>
        void load (Archive& ar, const unsigned int)
        {
          unique_lock_t lock (_mutex);

          ar & BOOST_SERIALIZATION_NVP(_id);
          ar & BOOST_SERIALIZATION_NVP(_flags);
          ar & BOOST_SERIALIZATION_NVP(_transition);

          load (ar, _pending_input);
          load (ar, _input);
          load (ar, _output);
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

      private:
        petri_net::activity_id_type _id;
        flags::flags_t _flags;
        mutable boost::recursive_mutex _mutex;

        we::type::transition_t _transition;

        input_t _pending_input;
        input_t _input;
        output_t _output;

        boost::mt19937 _engine;
      };

      bool operator== (const activity_t&, const activity_t&);
      std::ostream& operator<< (std::ostream&, const activity_t&);
    }
  }
}

#endif
