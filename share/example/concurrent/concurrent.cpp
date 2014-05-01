#include <we/loader/macros.hpp>
#include <drts/worker/context.hpp>

#include <fhglog/LogMacros.hpp>

#include <map>

static unsigned long call_cnt[4];

void fun (char c, expr::eval::context const& input, expr::eval::context& output)
{
  long const& id (boost::get<const long&> (input.value ("id")));

  ++call_cnt[c - 'A'];

  output.bind ("done", we::type::literal::control());
}

void A ( drts::worker::context*
       , expr::eval::context const& input
       , expr::eval::context& output
       )
{
  fun ('A', input, output);
}

void B ( drts::worker::context*
       , expr::eval::context const& input
       , expr::eval::context& output
       )
{
  fun ('B', input, output);
}

void C ( drts::worker::context*
       , expr::eval::context const& input
       , expr::eval::context& output
       )
{
  fun ('C', input, output);
}

void D ( drts::worker::context*
       , expr::eval::context const& input
       , expr::eval::context& output
       )
{
  fun ('D', input, output);
}

void finalize ( drts::worker::context*
              , expr::eval::context const&
              , expr::eval::context& output
              )
{
  output.bind ("done", we::type::literal::control());
}

WE_MOD_INITIALIZE_START (concurrent);
{
  for (char c ('A'); c < 'E'; ++c)
  {
    call_cnt[c - 'A'] = 0;
  }

  WE_REGISTER_FUN (A);
  WE_REGISTER_FUN (B);
  WE_REGISTER_FUN (C);
  WE_REGISTER_FUN (D);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (concurrent);

WE_MOD_FINALIZE_START (concurrent);
{
  for (char c ('A'); c < 'E'; ++c)
  {
    MLOG (INFO, "call_cnt_" << c << " = " << call_cnt[c - 'A']);
  }
}
WE_MOD_FINALIZE_END (concurrent);
