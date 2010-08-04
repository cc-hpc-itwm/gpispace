#include <we/loader/macros.hpp>

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

  value::structured_t b_start;
  value::structured_t b_end;
  value::structured_t b;

  b_start["x"] = stretch * a_start_x;
  b_start["y"] = stretch * a_start_y;
  b_end["x"] = stretch * a_end_x;
  b_end["y"] = stretch * get<double> (input, "a", "end.y");
  b["start"] = b_start;
  b["end"] = b_end;

  we::loader::put_output (output, "b", b);
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
