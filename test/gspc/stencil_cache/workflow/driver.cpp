#include <boost/test/unit_test.hpp>

#include <gspc/StencilCache.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <we/type/value/poke.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <test/gspc/stencil_cache/workflow/size.hpp>

#include <fhg/util/boost/program_options/validators/existing_path.hpp>

#include <util-generic/timer/application.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace test
{
  namespace gspc
  {
    namespace stencil_cache
    {
      BOOST_AUTO_TEST_CASE (stencil2D)
      {
        using ExistingPath = fhg::util::boost::program_options::existing_path;

        fhg::util::default_application_timer out {"Stencil2D"};

        out.section ("process command line and create temporary space");

        boost::program_options::options_description options_description;

        options_description.add_options()
          ( "neighbors"
          , boost::program_options::value<ExistingPath>()->required()
          , "path to implementation of neighbors"
          )
          ;
        options_description.add (::test::options::shared_directory());
        options_description.add (::test::options::source_directory());
        options_description.add (::gspc::options::installation());
        options_description.add (::gspc::options::drts());
        options_description.add (::gspc::options::logging());
        options_description.add (::gspc::options::scoped_rifd());
        options_description.add (::gspc::options::virtual_memory());

        boost::program_options::variables_map vm
          ( ::test::parse_command_line
            ( boost::unit_test::framework::master_test_suite().argc
            , boost::unit_test::framework::master_test_suite().argv
            , options_description
            )
          );

        fhg::util::temporary_path const shared_directory
          (::test::shared_directory (vm) / "stencil2D");

        test::scoped_nodefile_from_environment const nodefile_from_environment
          (shared_directory, vm);

        fhg::util::temporary_path const _installation_dir
          (shared_directory / boost::filesystem::unique_path());
        boost::filesystem::path const installation_dir (_installation_dir);

        ::gspc::set_application_search_path (vm, installation_dir);
        ::test::set_virtual_memory_socket_name_for_localhost (vm);

        vm.notify();

        boost::filesystem::path const neighbors
          (vm.at ("neighbors").as<ExistingPath>());

        ::gspc::installation const installation (vm);

        ::test::make_net_lib_install const make
          ( installation
          , "stencil_cache"
          , test::source_directory (vm)
          , installation_dir
          );

        ::gspc::scoped_rifds const rifds ( ::gspc::rifd::strategy {vm}
                                         , ::gspc::rifd::hostnames {vm}
                                         , ::gspc::rifd::port {vm}
                                         , installation
                                         );

        unsigned long const memory_per_node (3UL << 30UL);
        unsigned long const input_size (10UL << 20UL);
        unsigned long const compute_worker_per_node (4);
        unsigned long const load_worker_per_node (3);

        unsigned long const number_of_unique_nodes (rifds.hosts().size());

        unsigned long const communication_buffer_size (4 << 20);
        unsigned long const communication_buffer_per_node (8);

        long const X (workflow::size::X());
        long const Y (workflow::size::Y());
        long const R (workflow::size::R());
        long const H (2 * (R - 1) + 1);
        long const S (H * H);

        auto const worker_memory_per_node
          ( load_worker_per_node * input_size
          + compute_worker_per_node * S * input_size
          );
        auto const communication_buffer_memory_per_node
          (communication_buffer_per_node * communication_buffer_size);

        BOOST_REQUIRE_LT
          ( worker_memory_per_node + communication_buffer_memory_per_node
          , memory_per_node
          );

        auto const virtual_memory_per_node
          ( memory_per_node
          - worker_memory_per_node
          - communication_buffer_memory_per_node
          );

        auto const input_memory
          (virtual_memory_per_node * number_of_unique_nodes);

        ::gspc::scoped_runtime_system const drts
          ( vm
          , installation
          , ( boost::format ("load:%1%,%2%"
                            " compute:%3%,%4%"
                            )
            % load_worker_per_node
            % input_size
            % compute_worker_per_node
            % (S * input_size)
            ).str()
          , rifds.entry_points()
          , out
          );

        ::gspc::vmem_allocation const allocation_input
          ( drts.alloc ( ::gspc::vmem::gaspi_segment_description
                         ( communication_buffer_size
                         , communication_buffer_per_node
                         )
                       , input_memory
                       , "input"
                       )
          );

        auto const M (allocation_input.size() / input_size);
        long const B ((Y * H + M) / M);

        out << "memory size " << allocation_input.size() << '\n';
        out << "input size " << input_size << '\n';
        out << "X " << X
            << " Y " << Y
            << " R " << R
            << " M " << M
            << " B " << B
            << " C " << compute_worker_per_node
            << " L " << load_worker_per_node
            << '\n'
          ;

        auto Coordinate
          ( [] (long c)
            {
              pnet::type::value::value_type value;
              pnet::type::value::poke ("value", value, c);
              return value;
            }
          );
        auto Callback
          ( [] (boost::filesystem::path path)
            {
              pnet::type::value::value_type value;
              pnet::type::value::poke ("path", value, path.string());
              return value;
            }
          );

        auto const result
          ( ::gspc::client (drts).put_and_run
            ( ::gspc::workflow (make.pnet())
            , { {"X", Coordinate (X)}
              , {"Y", Coordinate (Y)}
              , {"B", B}
              , {"input_size", input_size}
              , {"input_memory", allocation_input.global_memory_range()}
              , {"neighbors", Callback (neighbors.string())}
              , {"cache_id", 0UL}
              }
            )
          );

        BOOST_REQUIRE_EQUAL (result.size(), 3);
        BOOST_REQUIRE_EQUAL (result.count ("done"), 1);
        BOOST_REQUIRE_EQUAL (result.count ("loads"), 1);
        BOOST_REQUIRE_EQUAL (result.count ("gets"), 1);

        out << "loads " << boost::get<unsigned long> (result.find ("loads")->second) << '\n';
        out << "gets " << boost::get<unsigned long> (result.find ("gets")->second) << '\n';
      }
    }
  }
}
