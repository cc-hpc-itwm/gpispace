// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_SIMPLE_PIPE_ELIMINATION_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_SIMPLE_PIPE_ELIMINATION_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>

#include <we/type/bits/transition/optimize/merge_places.hpp>
#include <we/type/bits/transition/optimize/is_associated.hpp>

#include <stack>
#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>

namespace we { namespace type {
    namespace optimize { namespace simple_pipe_elimination
    {
      // ******************************************************************* //

      struct pid_in_type
      {
        petri_net::pid_t pid;
        bool is_read;

        pid_in_type (const petri_net::pid_t & _pid, const bool & _is_read)
          : pid (_pid), is_read (_is_read)
        {}
      };

      struct pid_out_type
      {
        petri_net::pid_t pid;
        bool is_associated;

        pid_out_type (const petri_net::pid_t & _pid, const bool & _is_assoc)
          : pid (_pid), is_associated (_is_assoc)
        {}
      };

      struct pid_pair_type
      {
        pid_in_type in;
        pid_out_type out;

        pid_pair_type (const pid_in_type & _in, const pid_out_type & _out)
          : in (_in), out (_out)
        {}
      };

      namespace detail
      {
        template<typename Set>
        void insert_tids (Set& set, petri_net::adj_transition_const_it it)
        {
          for (; it.has_more(); ++it)
            {
              set.insert (*it);
            }
        }
      }

      typedef std::vector<pid_pair_type> pid_pair_vec_type;

      inline boost::optional<pid_pair_vec_type>
      pid_pairs ( const transition_t & trans
                , const petri_net::tid_t & tid
                , const petri_net::net<transition_t> & net
                , const transition_t & trans_parent
                )
      {
        typedef transition_t::port_t port_t;
        typedef port_t::name_type name_type;
        typedef petri_net::pid_t pid_t;
        typedef petri_net::eid_t eid_t;
        typedef petri_net::tid_t tid_t;
        typedef petri_net::connection_t connection_t;
        typedef transition_t::outer_to_inner_t outer_to_inner;
        typedef transition_t::inner_to_outer_t inner_to_outer;
        typedef boost::unordered_map<name_type, pid_t> map_type;
        typedef boost::unordered_set<tid_t> tid_set_type;

        map_type map_in;
        map_type map_out;

        for ( outer_to_inner::const_iterator
                oi (trans.outer_to_inner_begin())
            ; oi != trans.outer_to_inner_end()
            ; ++oi
            )
          {
            const port_t port (trans.get_port (oi->second.first));
            const pid_t pid (oi->first);

            map_in[port.name()] = pid;
          }

        for ( inner_to_outer::const_iterator
                io (trans.inner_to_outer_begin())
            ; io != trans.inner_to_outer_end()
            ; ++io
            )
          {
            const port_t port (trans.get_port (io->first));
            const pid_t pid (io->second.first);

            map_out[port.name()] = pid;
          }

        if (map_in.size() != map_out.size())
          {
            return boost::none;
          }

        pid_pair_vec_type pid_pair_vec;

        bool all_in_equals_one (true);
        bool all_out_equals_one (true);

        tid_set_type suc_in;
        tid_set_type suc_out;

        tid_set_type pred_in;
        tid_set_type pred_out;

        for ( map_type::const_iterator in (map_in.begin())
            ; in != map_in.end()
            ; ++in
            )
          {
            const map_type::const_iterator out
              (map_out.find (in->first));

            if (out == map_out.end())
              {
                return boost::none;
              }

            const petri_net::pid_t pid_A (in->second);
            const petri_net::pid_t pid_B (out->second);

            all_out_equals_one &= (net.out_of_place (pid_A).size() == 1);
            all_in_equals_one &= (net.in_to_place (pid_B).size() == 1);

            detail::insert_tids (suc_in, net.out_of_place (pid_A));
            detail::insert_tids (suc_out, net.out_of_place (pid_B));
            detail::insert_tids (pred_in, net.in_to_place (pid_A));
            detail::insert_tids (pred_out, net.in_to_place (pid_B));

            const eid_t eid (net.get_eid_in (tid, pid_A));

            if (petri_net::edge::is_pt_read (net.get_edge_info (eid).type))
              {
                return boost::none;
              }

            port_t port_A;
            port_t port_B;

            const bool ass_A (is_associated (trans_parent, pid_A, port_A));
            const bool ass_B (is_associated (trans_parent, pid_B, port_B));

            if (  (ass_A && port_A.is_input()  && ass_B && port_B.is_input() )
               || (ass_A && port_A.is_output() && ass_B && port_B.is_output())
               )
              {
                return boost::none;
              }

            if (  (( net.out_of_place (pid_A).size()
                   + ((ass_A && port_A.is_output()) ? 1 : 0)
                   ) > 1
                  )
               && (ass_B && port_B.is_output())
               )
              {
                return boost::none;
              }

            pid_pair_vec.push_back
              ( pid_pair_type
                ( pid_in_type ( pid_A
                              , net.is_read_connection (tid, pid_A)
                              )
                , pid_out_type (pid_B, ass_B)
                )
              );
          }

        if (!(all_in_equals_one || all_out_equals_one))
          {
            return boost::none;
          }
        else
          {
            for ( tid_set_type::const_iterator t (suc_in.begin())
                ; t != suc_in.end()
                ; ++t
                )
              {
                if (suc_out.find (*t) != suc_out.end())
                  {
                    return boost::none;
                  }
              }

            for ( tid_set_type::const_iterator t (pred_in.begin())
                ; t != pred_in.end()
                ; ++t
                )
              {
                if (pred_out.find (*t) != pred_out.end())
                  {
                    return boost::none;
                  }
              }
          }

        return pid_pair_vec;
      }

      // ******************************************************************* //

      inline bool run
      ( transition_t & trans_parent
      , petri_net::net<transition_t> & net
      )
      {
        typedef petri_net::net<transition_t> pnet_t;
        typedef pnet_t::transition_const_it transition_const_it;
        typedef petri_net::pid_t pid_t;
        typedef petri_net::tid_t tid_t;
        typedef transition_t::port_t port_t;

        bool modified (false);

        typedef std::stack<tid_t> stack_t;
        stack_t stack;

        for (transition_const_it t (net.transitions()); t.has_more(); ++t)
          {
            stack.push (*t);
          }

        while (!stack.empty())
          {
            const tid_t & tid (stack.top());
            const transition_t trans (net.get_transition (tid));

            if (  content::is_expression (trans)
               && boost::get<expression_t> (trans.data()).is_empty()
               && trans.condition().is_const_true()
               )
              {
                const boost::optional<pid_pair_vec_type>
                  pid_pair_vec (pid_pairs (trans, tid, net, trans_parent));

                if (pid_pair_vec)
                  {
                    net.delete_transition (tid);

                    for ( pid_pair_vec_type::const_iterator
                            pp ((*pid_pair_vec).begin())
                        ; pp != (*pid_pair_vec).end()
                        ; ++pp
                        )
                      {
                        const pid_pair_type & pid_pair (*pp);
                        const pid_in_type & in (pid_pair.in);
                        const pid_out_type & out (pid_pair.out);

                        merge_places (net, in.pid, in.is_read, out.pid);

                        if (out.is_associated)
                          {
                            trans_parent.UNSAFE_re_associate_port ( out.pid
                                                                  , in.pid
                                                                  );
                          }
                      }

                    modified = true;
                  }
              }

            stack.pop();
          }

        return modified;
      }
    }}
  }
}

#endif
