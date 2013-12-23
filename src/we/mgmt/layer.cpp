// mirko.rahn@itwm.fraunhofer.de

#include <we/mgmt/layer.hpp>

namespace we
{
  namespace mgmt
  {
    // BEGIN so soll das sein

    void layer::submit (detail::descriptor const &desc)
    {
      detail::descriptor desc (generate_internal_id(), act);
      desc.set_user_data (data);

      _nets_to_extract_from.push
        (is-net ? desc : net_t::net_with_only_child (desc));
    }

    void extract_from_nets()
    {
      while (true)
      {
        const net_t net (_nets_to_extract_from.get());

        BOOST_FOREACH (act, until_none (net.fire())) // only external
        {
          ext_submit (act);
          _awaiting_termination.push (net, act);
        }

        if (_awaiting_termination.contains (net, *)) // busy wait
        {
          _nets_to_extract_from.push (net);
        }
        else
        {
          ext_finished (net);
        }
      }
    }

    bool cleanup_if_no_childs_left_and_return_whether_there_are_some (net, after)
    {
      bool const has_childs (!net.children.empty());

      if (!has_childs)
      {
        _waiting_for_cancelation.remove (net);
        after (net);
      }

      return !has_childs;
    }

    void do_cancel (id, after)
    {
      net_t& net
        (_waiting_for_cancelation.push (_nets_to_extract_from.take (id)));

      if (cleanup_if_no_childs_left_and_return_whether_there_are_some (net, after))
      {
        BOOST_FOREACH (child, copy (net.children))
        {
          ext_cancel ( child_id
                     , [&](id)
                       {
                         // locked!
                         net.children.remove (id);
                         cleanup_if_no_childs_left_and_return_whether_there_are_some (net, after);
                       }
                     );
        }
      }
    }
    void cancel (id) // can only cancel top level
    {
      do_cancel (id, ext_canceled);
    }
    void failed (id) // reply to ext_submit -> childs only
    {
      assert (_awaiting_termination.contains (*, id));
      _awaiting_termination.remove (parent (id), id);
      do_cancel (parent (id), ext_failed);
    }
    void finished (id) // reply to ext_submit -> childs only
    {
      assert (_awaiting_termination.contains (*, id));
      _awaiting_termination.remove (*, id);
    }

    // END so soll das sein

  }
}
