#include <we/loader/macros.hpp>
#include <we/loader/putget.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

using we::loader::get;

static void run ( void *
                , const we::loader::input_t & input
                , we::loader::output_t & output
                )
{
  // *********************************************************************** //
  // get the complete token as literal value
  const double & stretch (get<double> (input, "stretch"));

  LOG (INFO, "stretch " << stretch);

  // also as value::type
  {
    const value::type & stretch_v (get<value::type> (input, "stretch"));

    LOG (INFO, "stretch_v " << stretch_v);

    // extract the literal from the subtoken
    const double & stretch_vd (we::loader::get<double> (stretch_v));

    LOG (INFO, "stretch_vd " << stretch_vd);
  }

  // *********************************************************************** //
  // get the complete token as value::type, structured
  const value::type & a (get<value::type> (input, "a"));

  LOG (INFO, "a " << a);

  // *********************************************************************** //
  // get a structured subtoken
  const value::type & a_start (get<value::type> (input, "a", "start"));

  LOG (INFO, "a_start " << a_start);

  // also by a value::path
  {
    value::path_type path; path.push_back ("start");

    LOG (INFO, "a_start (path) " << get<value::type> (input, "a", path));
  }

  // *********************************************************************** //
  // get a field from a subtoken
  const double & a_start_x (get<double> (a_start, "x"));

  LOG (INFO, "a_start_x " << a_start_x);

  // also by a value::path
  {
    value::path_type path; path.push_back ("x");

    LOG (INFO, "a_start_x (path) " << get<double> (a_start, path));
  }

  // *********************************************************************** //
  // get a path from a subtoken
  const double & a_start_y (get<double> (a, "start.y"));

   LOG (INFO, "a_start_y " << a_start_y);

  // also by a value::path
  {
    value::path_type path; path.push_back ("start"); path.push_back ("y");

    LOG (INFO, "a_start_y (path) " << get<double> (a, path));
  }

  // *********************************************************************** //
  // get a path from a token directly from the input
  const double & a_end_x (get<double> (input, "a", "end.x"));

  LOG (INFO, "a_end_x " << a_end_x);

  // also by a value::path
  {
    value::path_type path; path.push_back ("end"); path.push_back ("x");

    LOG (INFO, "a_end_x (path) " << get<double>(input, "a", path));
  }

  // *********************************************************************** //

  // not recommended
  value::structured_t b_start;

  b_start["x"] = stretch * a_start_x;
  b_start["y"] = stretch * a_start_y;

  // *********************************************************************** //
  // put a literal via a path on a port

  output.bind ("b.end.x", pnet::type::value::value_type (stretch * a_end_x));

  output.bind ("b.end.y", pnet::type::value::value_type (stretch * get<double> (input, "a", "end.y")));

  // *********************************************************************** //
  // put a complete subtoken via a path on a port

  output.bind ("b.start", value::type (b_start));

  // *********************************************************************** //
  // put a complete value on a port (here grab it from the already
  // constructed output)

  value::type b (get<value::type> (output, "b"));

  output.bind ("c.start", get<value::type> (b, "end"));
  output.bind ("c.end", get<value::type> (b, "start"));

}

WE_MOD_INITIALIZE_START (put_get);
{
  WE_REGISTER_FUN (run);
}
WE_MOD_INITIALIZE_END (put_get);

WE_MOD_FINALIZE_START (put_get);
{
}
WE_MOD_FINALIZE_END (put_get);
