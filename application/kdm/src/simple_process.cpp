#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <we/loader/macros.hpp>
#include <we/loader/putget.hpp>
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

// ************************************************************************* //

using we::loader::get;

// ************************************************************************* //

static void init ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  const std::string & filename (get<std::string> (input, "desc"));

  MLOG (INFO, "init: got filename " << filename);

  std::ifstream file (filename.c_str());

  if (!file)
    {
      throw std::runtime_error ("BUMMER: file not good");
    }

  MLOG (INFO, "init: read from " << filename);

  output.bind ("config.param.tpow.tpow", pnet::type::value::value_type (-1.0));
  output.bind ("config.param.clip.c", pnet::type::value::value_type (-1.0));
  output.bind ("config.param.trap.t", pnet::type::value::value_type (-1.0));
  output.bind ("config.param.bandpass.frequ1", pnet::type::value::value_type (-1.0));
  output.bind ("config.param.bandpass.frequ2", pnet::type::value::value_type (-1.0));
  output.bind ("config.param.bandpass.frequ3", pnet::type::value::value_type (-1.0));
  output.bind ("config.param.bandpass.frequ4", pnet::type::value::value_type (-1.0));
  output.bind ("config.exec.su", pnet::type::value::value_type (std::string("")));

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

              output.bind ("config." + s, pnet::type::value::value_type (v));
            }
	  else if (fhg::util::starts_with ("param", s))
	    {
              double v;
              file >> v;

              MLOG (INFO, "init: read " << s << " " << v);

              output.bind ("config." + s, pnet::type::value::value_type (v));
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

              output.bind ("config." + s, pnet::type::value::value_type (coll.substr(0,coll.size()-1)));
            }
          else
            {
              long v;
              file >> v;

              MLOG (INFO, "init: read " << s << " " << v);

              output.bind ("config." + s, pnet::type::value::value_type (v));
            }
        }
    }

  // determine size, overwrite values given in config file
  if (get<std::string> (output, "config", "input.type") != "text")
  {
    long num (0);
    long size (0);

    try
      {
        num = get<long> (output, "config", "trace_detect.number");
      }
    catch (...)
      {
        // do nothing, the value is not given
      }

    try
      {
        size = get<long> (output, "config", "trace_detect.size_in_bytes");
      }
    catch (...)
      {
        // do nothing, the value is not given
      }

    const std::string & t (get<std::string> (output, "config", "input.type"));

    if (t != "text")
      {
        const std::string & inp (get<std::string> (output, "config", "input.file"));

        determine_size (inp, t, num, size);

        output.bind ("config.trace_detect.number", pnet::type::value::value_type (num));
        output.bind ("config.trace_detect.size_in_bytes", pnet::type::value::value_type (size));
      }

    try
      {
	const long & slots_per_node (get<long> (output, "config", "tune.slots_per_node"));
	const long & memsize (get<long> (output, "config", "tune.memsize"));
	const long trace_per_bunch ((memsize/slots_per_node) / size);

	output.bind ("config.tune.trace_per_bunch", pnet::type::value::value_type (trace_per_bunch));
      }
    catch (...)
      {
	// do nothing, slots_per_node is not set
	output.bind ("config.tune.slots_per_node", pnet::type::value::value_type (1L));
      }
  }

  const long & trace_per_bunch (get<long> (output, "config", "tune.trace_per_bunch"));

  if (trace_per_bunch <= 0)
    {
      throw std::runtime_error ("BUMMER! trace_per_bunch <= 0");
    }

  const long & trace_num (get<long> (output, "config", "trace_detect.number"));
  const long & trace_size_in_bytes (get<long> (output, "config", "trace_detect.size_in_bytes"));
  const long & memsize (get<long> (output, "config", "tune.memsize"));

  const std::string & inputfile (get<std::string> (output, "config", "input.file"));
  const std::string & outputfile (get<std::string> (output, "config", "output.file"));

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

  if ( get<std::string> (output, "config", "output.type")
       !=
       get<std::string> (output, "config", "input.type")
     )
  {
    throw std::runtime_error ("Sorry, input type != output type not implemented yet!");
  }

  struct stat buffer;
  stat (inputfile.c_str(), &buffer);

  int outp_des (open (outputfile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));

  ftruncate (outp_des, buffer.st_size);

  close (outp_des);

  output.bind ("config.data.size", pnet::type::value::value_type (trace_size_in_bytes * trace_num));

  output.bind ("config.bunchbuffer.size", pnet::type::value::value_type (sizeofBunchBuffer));
  output.bind ("config.num.store", pnet::type::value::value_type ((num_slot_per_node - 1) * node_count));
  output.bind ("config.num.part", pnet::type::value::value_type (num_part));
  output.bind ("config.num.write_credit", pnet::type::value::value_type (node_count));
  output.bind ("config.num.load_credit", pnet::type::value::value_type (node_count));

  output.bind ("config.handle.data", pnet::type::value::value_type (static_cast<long>(handle_data)));
  output.bind ("config.handle.scratch", pnet::type::value::value_type (static_cast<long>(handle_scratch)));

  MLOG (INFO, "init: got config " << get<value::type>(output, "config"));
}

// ************************************************************************* //

static void finalize ( void * state
                     , const we::loader::input_t & input
                     , we::loader::output_t & output
                     )
{
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "finalize: config " << config);

  const fvmAllocHandle_t handle_data (get<long> (config, "handle.data"));
  const fvmAllocHandle_t handle_scratch (get<long> (config, "handle.scratch"));

  fvmGlobalFree (handle_data);
  fvmGlobalFree (handle_scratch);

  output.bind ("done", pnet::type::value::value_type (we::type::literal::control()));
}

// ************************************************************************* //

static void load ( void * state
		 , const we::loader::input_t & input
		 , we::loader::output_t & output
		 )
{
  const long & part (get<long> (input, "part"));
  const long & store (get<long> (input, "store"));
  const value::type & config (get<value::type> (input, "config"));

  MLOG (INFO, "load: part " << part << ", store " << store << ", config " << config);

  // load data from disk to fvmGetShmemPtr()

  const std::string & filename (get<std::string> (config, "input.file"));
  const std::string & type (get<std::string> (config, "input.type"));
  const long & sizeofBunchBuffer (get<long> (config, "bunchbuffer.size"));
  const long & data_size (get<long> (config, "data.size"));

  const long size (std::min ( sizeofBunchBuffer
                            , data_size - part * sizeofBunchBuffer
                            )
                  );

  const long & trace_size_in_bytes (get<long> (config, "trace_detect.size_in_bytes"));
  const long num (size / trace_size_in_bytes);

  do_load (filename, type, part, sizeofBunchBuffer, size, num, fvmGetShmemPtr());

  // communicate data to GPI space into handle.data using handle.scratch

  const fvmAllocHandle_t & handle_data (get<long> (config, "handle.data"));
  const fvmAllocHandle_t & handle_scratch (get<long> (config, "handle.scratch"));

  waitComm (fvmPutGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  output.bind ("part_loaded.id.part", pnet::type::value::value_type (part));
  output.bind ("part_loaded.id.store", pnet::type::value::value_type (store));
}

// ************************************************************************* //

static void write ( void * state
                  , const we::loader::input_t & input
                  , we::loader::output_t & output
                  )
{
  const value::type & config (get<value::type> (input, "config"));
  const long & part (get<long> (input, "part_in_store", "id.part"));
  const long & store (get<long> (input, "part_in_store", "id.store"));
  const we::type::literal::control & credit (get<we::type::literal::control> (input, "credit"));

  MLOG (INFO, "write: part " << part << ", store " << store << ", config " << config);

  // communicate from GPI space to fvmGetShmemPtr()

  const fvmAllocHandle_t & handle_data (get<long> (config, "handle.data"));
  const fvmAllocHandle_t & handle_scratch (get<long> (config, "handle.scratch"));
  const long & sizeofBunchBuffer (get<long> (config, "bunchbuffer.size"));
  const long & data_size (get<long> (config, "data.size"));

  const long size (std::min ( sizeofBunchBuffer
                            , data_size - part * sizeofBunchBuffer
                            )
                  );

  const long & trace_size_in_bytes (get<long> (config, "trace_detect.size_in_bytes"));
  const long num (size / trace_size_in_bytes);

  waitComm (fvmGetGlobalData ( handle_data
                             , store * sizeofBunchBuffer
                             , sizeofBunchBuffer
                             , 0
                             , handle_scratch
                             )
           );

  // save to disk from fvmGetShmemPtr()

  const std::string & filename (get<std::string> (config, "output.file"));
  const std::string & type (get<std::string> (config, "output.type"));

  do_write (filename, type, part, sizeofBunchBuffer, size, num, fvmGetShmemPtr());

  output.bind ("part", pnet::type::value::value_type (part));
  output.bind ("store", pnet::type::value::value_type (store));
  output.bind ("credit", pnet::type::value::value_type (credit));
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
