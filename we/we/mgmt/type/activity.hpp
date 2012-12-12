// {petry,rahn}@itwm.fhg.de

#ifndef WE_MGMT_TYPE_ACTIVITY_HPP
#define WE_MGMT_TYPE_ACTIVITY_HPP 1

#include <we/type/id.hpp>
#include <we/type/transition.hpp>

#include <we/mgmt/type/flags.hpp>
#include <we/mgmt/bits/transition_visitors.hpp>

#include <we/type/bits/transition/optimize.hpp>

#include <boost/unordered_set.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/serialization/access.hpp>
#include <boost/random.hpp>

#include <vector>

#include <iosfwd>

namespace we
{
  namespace mgmt
  {
    namespace type
    {
      //! \todo the whole principle of having two kinds of activities
      //! is broken, merge them! (the other is petri_net::net::activity_t)
      class activity_t
      {
      public:
        typedef std::pair<token::type, petri_net::port_id_type> token_on_port_t;
        typedef std::vector<token_on_port_t> token_on_port_list_t;
        typedef token_on_port_list_t input_t;
        typedef token_on_port_list_t output_t;

      private:
        typedef boost::unique_lock<boost::recursive_mutex> shared_lock_t;
        typedef boost::unique_lock<boost::recursive_mutex> unique_lock_t;

      public:
        activity_t ();
        activity_t (const we::type::transition_t&);
        activity_t (const activity_t&);

        activity_t& operator= (const activity_t&);

        void set_id (const petri_net::activity_id_type&);
        const petri_net::activity_id_type& id() const;
        const flags::flags_t& flags() const;

        bool is_alive() const;

#define FLAG(_name)                             \
        bool is_ ## _name() const;              \
        void set_ ## _name (bool value = true)

        FLAG (suspended);
        FLAG (cancelling);
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

        template <typename Context>
        typename Context::result_type execute (Context& ctxt)
        {
          unique_lock_t lock (_mutex);
          /* context requirements

             internal
             ========

             net:
             inject tokens into net
             ctxt.handle_internally (act, net)
             -> extractor

             expr:
             evaluate expression
             ctxt.handle_internally (act, expr)
             -> injector

             mod:
             prepare input
             [(token-on-place)], { place <-> name }
             ctxt.handle_internally (act, mod_call_t)

             external
             ========

             ctxt.handle_externally (act, net)
             ctxt.handle_externally (act, expr)
             ctxt.handle_externally (act, mod)

          */

          we::mgmt::visitor::executor<activity_t, Context> visitor_executor (*this, ctxt);
          return boost::apply_visitor (visitor_executor, transition().data());
        }

        bool can_fire() const;

        const input_t& pending_input() const;
        const input_t& input() const;
        void add_input (const input_t::value_type&);

        const output_t& output() const;
        void set_output (const output_t&);
        void add_output (const output_t::value_type&);

        std::ostream& print (std::ostream&, const token_on_port_list_t&) const;

      private:
        friend std::ostream& operator<< (std::ostream&, const activity_t&);
        void writeTo (std::ostream&) const;

        void lock();
        void unlock();

        friend class boost::serialization::access;
        template<class Archive>
        void serialize (Archive& ar, const unsigned int)
        {
          unique_lock_t lock (_mutex);

          ar & BOOST_SERIALIZATION_NVP(_id);
          ar & BOOST_SERIALIZATION_NVP(_flags);
          ar & BOOST_SERIALIZATION_NVP(_transition);
          ar & BOOST_SERIALIZATION_NVP(_pending_input);
          ar & BOOST_SERIALIZATION_NVP(_input);
          ar & BOOST_SERIALIZATION_NVP(_output);
        }

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
