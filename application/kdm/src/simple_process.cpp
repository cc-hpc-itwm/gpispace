#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <we/loader/macros.hpp>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/util.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include "SegYBHeader.h"
#include "SegYEBCHeader.h"
#include "SegYHeader.h"
#include "TraceBunch.hpp"
#include "TraceData.hpp"

#include <fhg/util/starts_with.hpp>

#include <determine_size.hpp>
#include <do_write.hpp>
#include <do_load.hpp>

#include <we2/type/value/peek.hpp>
#include <we2/type/value/show.hpp>

// ************************************************************************* //

namespace
{
  template<typename R>
    R peek (const pnet::type::value::value_type& x, const std::string& key)
  {
    return boost::get<R> (*pnet::type::value::peek (key, x));
  }
}

// ************************************************************************* //

static void init ( void * state
                 , const we::loader::input_t & input
                 , we::loader::output_t & output
                 )
{
  const std::string& filename (boost::get<const std::string&> (input.value ("desc")));

  MLOG (INFO, "init: got filename " << filename);

  std::ifstream file (filename.c_str());

  if (!file)
    {
      throw std::runtime_error ("BUMMER: file not good");
    }

  MLOG (INFO, "init: read from " << filename);

  output.bind ("config.param.tpow.tpow", -1.0);
  output.bind ("config.param.clip.c", -1.0);
  output.bind ("config.param.trap.t", -1.0);
  output.bind ("config.param.bandpass.frequ1", -1.0);
  output.bind ("config.param.bandpass.frequ2", -1.0);
  output.bind ("config.param.bandpass.frequ3", -1.0);
  output.bind ("config.param.bandpass.frequ4", -1.0);
  output.bind ("config.exec.su", std::string(""));

  while (!file.eof())
    {
      std::string s;
      file >> s;

      if (s.size())
        {
          if (  fhg::util::starts_with ("output", s)
	     || fhg::util::starts_with ("input", s)
	     )
            {
              std::string v;
              file >> v;

              MLOG (INFO, "init: read " << s << " " << v);

              output.bind ("config." + s, v);
            }
	  else if (fhg::util::starts_with ("param", s))
	    {
              double v;
              file >> v;

              MLOG (INFO, "init: read " << s << " " << v);

              output.bind ("config." + s, v);
	    }
          else if (fhg::util::starts_with ("exec", s))
            {
	      std::string coll;
	      std::string v;

	      file >> v;

	      assert (v[0] == '"');

	      coll = v.substr(1,v.size()-1);

	      while (v[v.size()-1] != '"')
		{
		  file >> v;

		  coll += ' ';
		  coll += v;
		}

              MLOG (INFO, "init: read " << s << " " << coll.substr(0,coll.size()-1));

              output.bind ("config." + s, coll.substr(0,coll.size()-1));
            }
          else
            {
              long v;
              file >> v;

              MLOG (INFO, "init: read " << s << " " << v);

              output.bind ("config." + s, v);
            }
        }
    }

  // determine size, overwrite values given in config file
  if (peek<std::string> (output.value("config"), "input.type") != "text")
  {
    long num (0);
    long size (0);

    try
      {
        num = peek<long> (output.value ("config"), "trace_detect.number");
      }
    catch (...)
      {
        // do nothing, the value is not given
      }

    try
      {
        size = peek<long> (output.value("config"), "trace_detect.size_in_bytes");
      }
    catch (...)
      {
        // do nothing, the value is not given
      }

    const std::string & t (peek<const std::string&> (output.value ("config"), "input.type"));

    if (t != "text")
      {
        const std::string& inp (peek<const std::string&> (output.value ("config"), "input.file"));

        determine_size (inp, t, num, size);

        output.bind ("config.trace_detect.number", num);
        output.bind ("config.trace_detect.size_in_bytes", size);
      }

    try
      {
        const long & slots_per_node (peek<const long&> (output.value ("config"), "tune.slots_per_node"));
        const long & memsize (peek<const long&> (output.value ("config"), "tune.memsize"));
	const long trace_per_bunch ((memsize/slots_per_node) / size);

	output.bind ("config.tune.trace_per_bunch", trace_per_bunch);
      }
    catch (...)
      {
	// do nothing, slots_per_node is not set
	output.bind ("config.tune.slots_per_node", 1L);
      }
  }

  const long& trace_per_bunch (peek<const long&> (output.value ("config"), "tune.trace_per_bunch"));

  if (trace_per_bunch <= 0)
    {
      throw std::runtime_error ("BUMMER! trace_per_bunch <= 0");
    }

  const long& trace_num (peek<const long&> (output.value("config"), "trace_detect.number"));
  const long& trace_size_in_bytes (peek<const long&> (output.value("config"), "trace_detect.size_in_bytes"));
  const long & memsize (peek<const long&> (output.value("config"), "tune.memsize"));

  const std::string& inputfile (peek<const std::string&> (output.value("config"), "input.file"));
  const std::string& outputfile (peek<const std::string&> (output.value("config"), "output.file"));

  const long sizeofBunchBuffer (trace_per_bunch * trace_size_in_bytes);

  MLOG (INFO, "init: sizeofBunchBuffer " << sizeofBunchBuffer);

  const long node_count (fvmGetNodeCount());
  const long num_slot_per_node (memsize / sizeofBunchBuffer);

  MLOG (INFO, "init: num_slot_per_node " << num_slot_per_node);

  const fvmAllocHandle_t handle_data
    (fvmGlobalAlloc ((num_slot_per_node - 1) * sizeofBunchBuffer));
  if (handle_data == 0)
    {
      throw std::runtime_error ("BUMMER! handle_data == 0");
    }

  MLOG (INFO, "init: handle_data " << handle_data << ", bytes " << (num_slot_per_node - 1) * sizeofBunchBuffer);

  const fvmAllocHandle_t handle_scratch
    (fvmGlobalAlloc (1 * sizeofBunchBuffer));
  if (handle_scratch == 0)
    {
      throw std::runtime_error ("BUMMER! handle_scratch == 0");
    }

  MLOG (INFO, "init: handle_scratch " << handle_scratch << ", bytes " << sizeofBunchBuffer);

  long num_part (trace_num / trace_per_bunch);

  if (trace_num % trace_per_bunch != 0)
    {
      num_part += 1;
    }

  if ( peek<std::string> (output.value("config"), "output.type")
       !=
       peek<std::string> (output.value("config"), "input.type")
     )
  {
    throw std::runtime_error ("Sorry, input type != output type not implemented yet!");
  }

  struct stat buffer;
  stat (inputfile.c_str(), &buffer);

  int outp_des (open (outputfile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));

  ftruncate (outp_des, buffer.st_size);

  close (outp_des);

  output.bind ("config.data.size", trace_size_in_bytes * trace_num);

  output.bind ("config.bunchbuffer.size", sizeofBunchBuffer);
  output.bind ("config.num.store", (num_slot_per_node - 1) * node_count);
  output.bind ("config.num.part", num_part);
  output.bind ("config.num.write_credit", node_count);
  output.bind ("config.num.load_credit", node_count);

  output.bind ("config.handle.data", static_cast<long>(handle_data));
  output.bind ("config.handle.scratch", static_cast<long>(handle_scratch));

  MLOG (INFO, "init: got config " << pnet::type::value::show (output.value ("config")));
}

// ************************************************************************* //

static void finalize ( void * state
                     , const we::loader::input_t & input
                     , we::loader::output_t & output
                     )
{
  const pnet::type::value::value_type& config (input.value("config"));

  MLOG (INFO, "finalize: config " << pnet::type::value::show (config));

  const fvmAllocHandle_t handle_data (peek<long> (config, "handle.data"));
  const fvmAllocHandle_t handle_scratch (peek<long> (config, "handle.scratch"));

  fvmGlobalFree (handle_data);
  fvmGlobalFree (handle_scratch);

  output.bind ("done", we::type::literal::control());
}

// ************************************************************************* //

static void load ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  const long & part (boost::get<const long&> (input.value ("part")));
  const long & store (boost::get<const long&> (input.value ("store")));
  const pnet::type::value::value_type & config (input.value ("config"));

  MLOG (INFO, "load: part " << part << ", store " << store << ", config " << pnet::type::value::show (config));

  // load data from disk to fvmGetShmemPtr()

  const std::string & filename (peek<const std::string&> (config, "input.file"));
  const std::string & type (peek<const std::string&> (config, "input.type"));
  const long & sizeofBunchBuffer (peek<const long&> (config, "bunchbuffer.size"));
  const long & data_size (peek<const long&> (config, "data.size"));

  const long size (std::min ( sizeofBunchBuffer
                            , data_size - part * sizeofBunchBuffer
                            )
                  );

  const long & trace_size_in_bytes (peek<const long&> (config, "trace_detect.size_in_bytes"));
  const long num (size / trace_size_in_bytes);

  do_load (filename, type, part, sizeofBunchBuffer, size, num, fvmGetShmemPtr());

  // communicate data to GPI space into handle.data using handle.scratch

  const fvmAllocHandle_t handle_data (peek<long> (config, "handle.data"));
  const fvmAllocHandle_t handle_scratch (peek<long> (config, "handle.scratch"));

  waitComm (fvmPutGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  output.bind ("part_loaded.id.part", part);
  output.bind ("part_loaded.id.store", store);
}

// ************************************************************************* //

static void write ( void * state
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  const pnet::type::value::value_type& config (input.value( "config"));
  const long & part (peek<const long&> (input.value("part_in_store"), "id.part"));
  const long & store (peek<const long&> (input.value("part_in_store"), "id.store"));

  MLOG (INFO, "write: part " << part << ", store " << store << ", config " << pnet::type::value::show (config));

  // communicate from GPI space to fvmGetShmemPtr()

  const fvmAllocHandle_t handle_data (peek<long> (config, "handle.data"));
  const fvmAllocHandle_t handle_scratch (peek<long> (config, "handle.scratch"));
  const long sizeofBunchBuffer (peek<long> (config, "bunchbuffer.size"));
  const long data_size (peek<long> (config, "data.size"));

  const long size (std::min ( sizeofBunchBuffer
                            , data_size - part * sizeofBunchBuffer
                            )
                  );

  const long trace_size_in_bytes (peek<long> (config, "trace_detect.size_in_bytes"));
  const long num (size / trace_size_in_bytes);

  waitComm (fvmGetGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  // save to disk from fvmGetShmemPtr()

  const std::string filename (peek<std::string> (config, "output.file"));
  const std::string type (peek<std::string> (config, "output.type"));

  do_write (filename, type, part, sizeofBunchBuffer, size, num, fvmGetShmemPtr());

  output.bind ("part", part);
  output.bind ("store", store);
  output.bind ("credit", we::type::literal::control());
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (simple_process);
{
  LOG(INFO, "WE_MOD_INITIALIZE_START (simple_process)");

  WE_REGISTER_FUN (init);
  WE_REGISTER_FUN (load);
  WE_REGISTER_FUN (write);
  WE_REGISTER_FUN (finalize);
}
WE_MOD_INITIALIZE_END (simple_process);

WE_MOD_FINALIZE_START (simple_process);
{
  LOG(INFO, "WE_MOD_FINALIZE_START (simple_process)");
}
WE_MOD_FINALIZE_END (simple_process);
