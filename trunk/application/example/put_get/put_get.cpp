#include <we/loader/macros.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

static void run ( void *
                , const we::loader::input_t & input
                , we::loader::output_t & output
                )
{
  const value::type & a (input.value ("a"));

  LOG (INFO, "a " << a);

  const value::type & a_start (value::get<value::type> ("start", a));

  LOG (INFO, "a_start " << a_start);

  const double & a_start_x (value::get<double> ("x", a_start));
  const double & a_start_y (value::get<double> ("y", a_start));
  const double & a_end_x (value::get<double> ("end.x", a));
  const double & a_end_y (value::get<double> ("end.y", a));

  LOG (INFO, "a_start_x " << a_start_x);
  LOG (INFO, "a_start_y " << a_start_y);
  LOG (INFO, "a_end_x " << a_end_x);
  LOG (INFO, "a_end_y " << a_end_y);

  const double & stretch (we::loader::get_input<double> (input, "stretch"));

  LOG (INFO, "stretch " << stretch);

  value::structured_t b_start;
  value::structured_t b_end;
  value::structured_t b;

  b_start["x"] = stretch * a_start_x;
  b_start["y"] = stretch * a_start_y;
  b_end["x"] = stretch * a_end_x;
  b_end["y"] = stretch * a_end_y;
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
