// bernd.loerwald@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_evaluate_net

#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>
#include <we/mgmt/context.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/net.hpp>
#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/util/token.hpp>

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
  class fixture : public we::mgmt::context
  {
  public:
    fixture()
    {
      BOOST_REQUIRE (boost::unit_test::framework::master_test_suite().argc >= 2);
      path_to_pnets = boost::unit_test::framework::master_test_suite().argv[1];
    }

    void load_activity_from_file (const boost::filesystem::path& path)
    {
      activity = we::mgmt::type::activity_t (path_to_pnets / path);
    }

    void put_token_from_string
      (const std::string& port, const std::string& value)
    {
      std::size_t k (0);
      std::string::const_iterator begin (value.begin());
      fhg::util::parse::position pos (k, begin, value.end());

      put_token (port, ::value::read (pos));
    }

    void put_token (const std::string& port, const value::type& value)
    {
      we::util::token::put (activity, port, value);
    }

    //! \todo Get token and compare value.

    void execute()
    {
      activity.inject_input();
      activity.execute (this);
      activity.collect_output();
    }

    const we::mgmt::type::activity_t::output_t& output() const
    {
      return activity.output();
    }


    // std::string content_of_file (const boost::filesystem::path& path) const
    // {
    //   std::ifstream reference_file
    //     ((path_to_pnets / path).string().c_str(), std::ios::in);

    //   BOOST_REQUIRE (reference_file);

    //   std::string reference;
    //   reference_file.seekg (0, std::ios::end);
    //   reference.resize (reference_file.tellg());
    //   reference_file.seekg (0, std::ios::beg);
    //   reference_file.read (&reference[0], reference.size());
    //   reference_file.close();

    //   return reference;
    // }

    //! \note Context copied from we-eval.
    virtual int handle_internally (we::mgmt::type::activity_t& act, net_t&)
    {
      act.inject_input ();

      while (act.can_fire())
      {
        we::mgmt::type::activity_t sub (act.extract());
        sub.inject_input ();
        sub.execute (this);
        act.inject (sub);
      }

      act.collect_output ();

      return 0;
    }

    virtual int handle_internally (we::mgmt::type::activity_t& act, mod_t& mod)
    {
      module::call (loader, act, mod);

      return 0;
    }

    virtual int handle_internally (we::mgmt::type::activity_t& , expr_t&)
    {
      return 0;
    }

    virtual int handle_externally (we::mgmt::type::activity_t& act, net_t& n)
    {
      return handle_internally (act, n);
    }

    virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
    {
      return handle_internally (act, mod);
    }

    virtual int handle_externally (we::mgmt::type::activity_t& act, expr_t& e)
    {
      return handle_internally (act, e);
    }

  protected:
    we::loader::loader loader;
    we::mgmt::type::activity_t activity;
    boost::filesystem::path path_to_pnets;
  };
}

// BOOST_FIXTURE_TEST_CASE (example, fixture)
// {
//   loader.load (module);
//   loader.append_search_path (path);

//   load_activity_from_file ("token.pnet");

//   put_token_from_string ("i", "[]");
//   put_token ("i", value::type (1));

//   execute();

//   BOOST_REQUIRE_EQUAL (content_of_file (path), activity.to_string());
// }

BOOST_TEST_DONT_PRINT_LOG_VALUE(we::mgmt::type::activity_t::output_t);

BOOST_FIXTURE_TEST_CASE (sequence, fixture)
{
  {
    load_activity_from_file ("sequence_input.pnet");

    put_token_from_string ("n", "1");

    execute();

    we::mgmt::type::activity_t::output_t out (output());

    load_activity_from_file ("sequence_output_1.pnet");

    BOOST_REQUIRE_EQUAL (out, output());
  }

  {
    load_activity_from_file ("sequence_input.pnet");

    put_token_from_string ("n", "10");

    execute();

    we::mgmt::type::activity_t::output_t out (output());

    load_activity_from_file ("sequence_output_10.pnet");

    BOOST_REQUIRE_EQUAL (out, output());
  }
}
