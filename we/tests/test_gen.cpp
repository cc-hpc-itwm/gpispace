#include <iostream>
#include <we/type/value/show.hpp>

#include <we/type/bitsetofint.hpp>
#include <we/type/literal/control.hpp>
#include <we/type/value.hpp>
#include <we/type/value/cpp/get.hpp>
#include <string>

namespace pnetc
{
  namespace kdm
  {

    namespace config
    {
      // defined in application/kdm/pnet/full/type/config.xml

      struct config {
        struct {
          long most;
        } assign;
        struct {
          long TT;
          long bunch;
          long job;
          long volume;
        } handle;
        struct {
          long parallel;
        } loadTT;
        long offsets;
        struct {
          struct {
            long bunches;
            long volumes;
          } offset;
          struct {
            long copies;
          } volume;
        } per;
        struct {
          long bunch;
          long job;
          long volume;
        } scratch;
        struct {
          struct {
            long bunch;
            long volume;
          } store;
        } size;
        struct {
          long N;
        } threads;
      };
      config from_value (const value::type & v_2)
      {
        config x;
        {
          const value::type & v_3 (value::get_level ("assign", v_2));
          {
            const value::type & v_4 (value::get_level ("most", v_3));
            x.assign.most = value::get<long> (v_4);
          }
        }
        {
          const value::type & v_3 (value::get_level ("handle", v_2));
          {
            const value::type & v_4 (value::get_level ("TT", v_3));
            x.handle.TT = value::get<long> (v_4);
          }
          {
            const value::type & v_4 (value::get_level ("bunch", v_3));
            x.handle.bunch = value::get<long> (v_4);
          }
          {
            const value::type & v_4 (value::get_level ("job", v_3));
            x.handle.job = value::get<long> (v_4);
          }
          {
            const value::type & v_4 (value::get_level ("volume", v_3));
            x.handle.volume = value::get<long> (v_4);
          }
        }
        {
          const value::type & v_3 (value::get_level ("loadTT", v_2));
          {
            const value::type & v_4 (value::get_level ("parallel", v_3));
            x.loadTT.parallel = value::get<long> (v_4);
          }
        }
        {
          const value::type & v_3 (value::get_level ("offsets", v_2));
          x.offsets = value::get<long> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("per", v_2));
          {
            const value::type & v_4 (value::get_level ("offset", v_3));
            {
              const value::type & v_5 (value::get_level ("bunches", v_4));
              x.per.offset.bunches = value::get<long> (v_5);
            }
            {
              const value::type & v_5 (value::get_level ("volumes", v_4));
              x.per.offset.volumes = value::get<long> (v_5);
            }
          }
          {
            const value::type & v_4 (value::get_level ("volume", v_3));
            {
              const value::type & v_5 (value::get_level ("copies", v_4));
              x.per.volume.copies = value::get<long> (v_5);
            }
          }
        }
        {
          const value::type & v_3 (value::get_level ("scratch", v_2));
          {
            const value::type & v_4 (value::get_level ("bunch", v_3));
            x.scratch.bunch = value::get<long> (v_4);
          }
          {
            const value::type & v_4 (value::get_level ("job", v_3));
            x.scratch.job = value::get<long> (v_4);
          }
          {
            const value::type & v_4 (value::get_level ("volume", v_3));
            x.scratch.volume = value::get<long> (v_4);
          }
        }
        {
          const value::type & v_3 (value::get_level ("size", v_2));
          {
            const value::type & v_4 (value::get_level ("store", v_3));
            {
              const value::type & v_5 (value::get_level ("bunch", v_4));
              x.size.store.bunch = value::get<long> (v_5);
            }
            {
              const value::type & v_5 (value::get_level ("volume", v_4));
              x.size.store.volume = value::get<long> (v_5);
            }
          }
        }
        {
          const value::type & v_3 (value::get_level ("threads", v_2));
          {
            const value::type & v_4 (value::get_level ("N", v_3));
            x.threads.N = value::get<long> (v_4);
          }
        }
        return x;
      }
      value::type to_value (const config & x)
      {
        value::structured_t v_3;
        {
          value::structured_t v_4;
          v_4["most"] = x.assign.most;
          v_3["assign"] = v_4;
        }
        {
          value::structured_t v_4;
          v_4["TT"] = x.handle.TT;
          v_4["bunch"] = x.handle.bunch;
          v_4["job"] = x.handle.job;
          v_4["volume"] = x.handle.volume;
          v_3["handle"] = v_4;
        }
        {
          value::structured_t v_4;
          v_4["parallel"] = x.loadTT.parallel;
          v_3["loadTT"] = v_4;
        }
        v_3["offsets"] = x.offsets;
        {
          value::structured_t v_4;
          {
            value::structured_t v_5;
            v_5["bunches"] = x.per.offset.bunches;
            v_5["volumes"] = x.per.offset.volumes;
            v_4["offset"] = v_5;
          }
          {
            value::structured_t v_5;
            v_5["copies"] = x.per.volume.copies;
            v_4["volume"] = v_5;
          }
          v_3["per"] = v_4;
        }
        {
          value::structured_t v_4;
          v_4["bunch"] = x.scratch.bunch;
          v_4["job"] = x.scratch.job;
          v_4["volume"] = x.scratch.volume;
          v_3["scratch"] = v_4;
        }
        {
          value::structured_t v_4;
          {
            value::structured_t v_5;
            v_5["bunch"] = x.size.store.bunch;
            v_5["volume"] = x.size.store.volume;
            v_4["store"] = v_5;
          }
          v_3["size"] = v_4;
        }
        {
          value::structured_t v_4;
          v_4["N"] = x.threads.N;
          v_3["threads"] = v_4;
        }
        return v_3;
      }
    } // config

    namespace store_bunch
    {
      // defined in application/kdm/pnet/full/type/store/bunch.xml

      struct store_bunch {
        long id;
      };
      store_bunch from_value (const value::type & v_2)
      {
        store_bunch x;
        {
          const value::type & v_3 (value::get_level ("id", v_2));
          x.id = value::get<long> (v_3);
        }
        return x;
      }
      value::type to_value (const store_bunch & x)
      {
        value::structured_t v_3;
        v_3["id"] = x.id;
        return v_3;
      }
    } // store_bunch

    namespace store_volume
    {
      // defined in application/kdm/pnet/full/type/store/volume.xml

      struct store_volume {
        long id;
      };
      store_volume from_value (const value::type & v_2)
      {
        store_volume x;
        {
          const value::type & v_3 (value::get_level ("id", v_2));
          x.id = value::get<long> (v_3);
        }
        return x;
      }
      value::type to_value (const store_volume & x)
      {
        value::structured_t v_3;
        v_3["id"] = x.id;
        return v_3;
      }
    } // store_volume

    namespace n_of_m
    {
      // defined in application/kdm/pnet/full/type/n_of_m.xml

      struct n_of_m {
        long id;
        long max;
      };
      n_of_m from_value (const value::type & v_2)
      {
        n_of_m x;
        {
          const value::type & v_3 (value::get_level ("id", v_2));
          x.id = value::get<long> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("max", v_2));
          x.max = value::get<long> (v_3);
        }
        return x;
      }
      value::type to_value (const n_of_m & x)
      {
        value::structured_t v_3;
        v_3["id"] = x.id;
        v_3["max"] = x.max;
        return v_3;
      }
    } // n_of_m

    namespace bunch
    {
      // defined in application/kdm/pnet/full/type/bunch.xml

      struct bunch {
        long id;
        struct {
          long id;
        } offset;
      };
      bunch from_value (const value::type & v_2)
      {
        bunch x;
        {
          const value::type & v_3 (value::get_level ("id", v_2));
          x.id = value::get<long> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("offset", v_2));
          {
            const value::type & v_4 (value::get_level ("id", v_3));
            x.offset.id = value::get<long> (v_4);
          }
        }
        return x;
      }
      value::type to_value (const bunch & x)
      {
        value::structured_t v_3;
        v_3["id"] = x.id;
        {
          value::structured_t v_4;
          v_4["id"] = x.offset.id;
          v_3["offset"] = v_4;
        }
        return v_3;
      }
    } // bunch

    namespace bunch_with_store
    {
      // defined in application/kdm/pnet/full/type/bunch/with_store.xml

      struct bunch_with_store {
        bunch::bunch bunch;
        store_bunch::store_bunch store;
        struct {
          struct {
            bitsetofint::type id;
          } seen;
        } volumes;
      };
      bunch_with_store from_value (const value::type & v_2)
      {
        bunch_with_store x;
        {
          const value::type & v_3 (value::get_level ("bunch", v_2));
          x.bunch = bunch::from_value (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("store", v_2));
          x.store = store_bunch::from_value (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("volumes", v_2));
          {
            const value::type & v_4 (value::get_level ("seen", v_3));
            {
              const value::type & v_5 (value::get_level ("id", v_4));
              x.volumes.seen.id = value::get<bitsetofint::type> (v_5);
            }
          }
        }
        return x;
      }
      value::type to_value (const bunch_with_store & x)
      {
        value::structured_t v_3;
        v_3["bunch"] = bunch::to_value (x.bunch);
        v_3["store"] = store_bunch::to_value (x.store);
        {
          value::structured_t v_4;
          {
            value::structured_t v_5;
            v_5["id"] = x.volumes.seen.id;
            v_4["seen"] = v_5;
          }
          v_3["volumes"] = v_4;
        }
        return v_3;
      }
    } // bunch_with_store

    namespace bunch_loaded
    {
      // defined in application/kdm/pnet/full/type/bunch/loaded.xml

      struct bunch_loaded {
        bunch_with_store::bunch_with_store bunch;
        long wait;
      };
      bunch_loaded from_value (const value::type & v_2)
      {
        bunch_loaded x;
        {
          const value::type & v_3 (value::get_level ("bunch", v_2));
          x.bunch = bunch_with_store::from_value (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("wait", v_2));
          x.wait = value::get<long> (v_3);
        }
        return x;
      }
      value::type to_value (const bunch_loaded & x)
      {
        value::structured_t v_3;
        v_3["bunch"] = bunch_with_store::to_value (x.bunch);
        v_3["wait"] = x.wait;
        return v_3;
      }
    } // bunch_loaded

    namespace volume
    {
      // defined in application/kdm/pnet/full/type/volume.xml

      struct volume {
        long id;
        struct {
          long id;
        } offset;
      };
      volume from_value (const value::type & v_2)
      {
        volume x;
        {
          const value::type & v_3 (value::get_level ("id", v_2));
          x.id = value::get<long> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("offset", v_2));
          {
            const value::type & v_4 (value::get_level ("id", v_3));
            x.offset.id = value::get<long> (v_4);
          }
        }
        return x;
      }
      value::type to_value (const volume & x)
      {
        value::structured_t v_3;
        v_3["id"] = x.id;
        {
          value::structured_t v_4;
          v_4["id"] = x.offset.id;
          v_3["offset"] = v_4;
        }
        return v_3;
      }
    } // volume

    namespace volume_state
    {
      // defined in application/kdm/pnet/full/type/volume/state.xml

      struct volume_state {
        struct {
          long left;
        } bunches;
        long copies;
        volume::volume volume;
      };
      volume_state from_value (const value::type & v_2)
      {
        volume_state x;
        {
          const value::type & v_3 (value::get_level ("bunches", v_2));
          {
            const value::type & v_4 (value::get_level ("left", v_3));
            x.bunches.left = value::get<long> (v_4);
          }
        }
        {
          const value::type & v_3 (value::get_level ("copies", v_2));
          x.copies = value::get<long> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("volume", v_2));
          x.volume = volume::from_value (v_3);
        }
        return x;
      }
      value::type to_value (const volume_state & x)
      {
        value::structured_t v_3;
        {
          value::structured_t v_4;
          v_4["left"] = x.bunches.left;
          v_3["bunches"] = v_4;
        }
        v_3["copies"] = x.copies;
        v_3["volume"] = volume::to_value (x.volume);
        return v_3;
      }
    } // volume_state

    namespace volume_with_store
    {
      // defined in application/kdm/pnet/full/type/volume/with_store.xml

      struct volume_with_store {
        store_volume::store_volume store;
        volume::volume volume;
      };
      volume_with_store from_value (const value::type & v_2)
      {
        volume_with_store x;
        {
          const value::type & v_3 (value::get_level ("store", v_2));
          x.store = store_volume::from_value (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("volume", v_2));
          x.volume = volume::from_value (v_3);
        }
        return x;
      }
      value::type to_value (const volume_with_store & x)
      {
        value::structured_t v_3;
        v_3["store"] = store_volume::to_value (x.store);
        v_3["volume"] = volume::to_value (x.volume);
        return v_3;
      }
    } // volume_with_store

    namespace volume_in_progress
    {
      // defined in application/kdm/pnet/full/type/volume/in_progress.xml

      struct volume_in_progress {
        struct {
          struct {
            long id;
          } bunch;
          struct {
            long id;
          } store;
        } assigned;
        volume_with_store::volume_with_store volume;
      };
      volume_in_progress from_value (const value::type & v_2)
      {
        volume_in_progress x;
        {
          const value::type & v_3 (value::get_level ("assigned", v_2));
          {
            const value::type & v_4 (value::get_level ("bunch", v_3));
            {
              const value::type & v_5 (value::get_level ("id", v_4));
              x.assigned.bunch.id = value::get<long> (v_5);
            }
          }
          {
            const value::type & v_4 (value::get_level ("store", v_3));
            {
              const value::type & v_5 (value::get_level ("id", v_4));
              x.assigned.store.id = value::get<long> (v_5);
            }
          }
        }
        {
          const value::type & v_3 (value::get_level ("volume", v_2));
          x.volume = volume_with_store::from_value (v_3);
        }
        return x;
      }
      value::type to_value (const volume_in_progress & x)
      {
        value::structured_t v_3;
        {
          value::structured_t v_4;
          {
            value::structured_t v_5;
            v_5["id"] = x.assigned.bunch.id;
            v_4["bunch"] = v_5;
          }
          {
            value::structured_t v_5;
            v_5["id"] = x.assigned.store.id;
            v_4["store"] = v_5;
          }
          v_3["assigned"] = v_4;
        }
        v_3["volume"] = volume_with_store::to_value (x.volume);
        return v_3;
      }
    } // volume_in_progress

    namespace volume_in_reduction
    {
      // defined in application/kdm/pnet/full/type/volume/in_reduction.xml

      struct volume_in_reduction {
        long left;
        volume_with_store::volume_with_store volume;
      };
      volume_in_reduction from_value (const value::type & v_2)
      {
        volume_in_reduction x;
        {
          const value::type & v_3 (value::get_level ("left", v_2));
          x.left = value::get<long> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("volume", v_2));
          x.volume = volume_with_store::from_value (v_3);
        }
        return x;
      }
      value::type to_value (const volume_in_reduction & x)
      {
        value::structured_t v_3;
        v_3["left"] = x.left;
        v_3["volume"] = volume_with_store::to_value (x.volume);
        return v_3;
      }
    } // volume_in_reduction

    namespace pair_to_reduce
    {
      // defined in application/kdm/pnet/full/type/volume/pair_to_reduce.xml

      struct pair_to_reduce {
        volume_in_reduction::volume_in_reduction sum;
        volume_with_store::volume_with_store vol;
      };
      pair_to_reduce from_value (const value::type & v_2)
      {
        pair_to_reduce x;
        {
          const value::type & v_3 (value::get_level ("sum", v_2));
          x.sum = volume_in_reduction::from_value (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("vol", v_2));
          x.vol = volume_with_store::from_value (v_3);
        }
        return x;
      }
      value::type to_value (const pair_to_reduce & x)
      {
        value::structured_t v_3;
        v_3["sum"] = volume_in_reduction::to_value (x.sum);
        v_3["vol"] = volume_with_store::to_value (x.vol);
        return v_3;
      }
    } // pair_to_reduce
  } // kdm
} // pnetc

int main ()
{
  pnetc::kdm::store_bunch::store_bunch s; s.id = 23;
  pnetc::kdm::bunch::bunch b; b.id = 42; b.offset.id = 13;
  pnetc::kdm::bunch_with_store::bunch_with_store bws;
  bws.bunch = b;
  bws.store = s;
  bws.volumes.seen.id = bitsetofint::type();
  bws.volumes.seen.id.ins (15);
  bws.volumes.seen.id.ins (15+64);

  std::cout << pnetc::kdm::bunch_with_store::to_value (bws) << std::endl;
}
