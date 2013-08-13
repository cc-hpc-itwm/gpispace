#include <limits>

//#include <pnetc/op/asian/run.hpp>

#include <sstream>
#include <process/process.hpp>

#include <boost/thread.hpp>

namespace asian
{
  static void fun ( ::pnetc::type::sums::type& sums
                  , const int tid
                  )
  {
    try
    {
      std::ostringstream str;

      str << "S = " << sums.param.S << std::endl;
      str << "K = " << sums.param.K << std::endl;
      str << "T = " << sums.param.T << std::endl;
      str << "sigma = " << sums.param.sigma << std::endl;
      str << "r = " << sums.param.r << std::endl;
      str << "d = " << sums.param.d << std::endl;
      str << "FirstFixing = " << sums.param.FirstFixing << std::endl;
      str << "FixingsProJahr = " << sums.param.FixingsProJahr << std::endl;
      str << "AnzahlderDividende = " << sums.param.AnzahlderDividende << std::endl;
      str << "n = " << sums.param.iterations_per_run << std::endl;

      const std::size_t s (str.str().size());

      char result[1024];

      const std::size_t written
        ( process::execute ( sums.param.bin
                           , str.str().c_str()
                           , s
                           , result
                           , 1024
                           )
        );

      std::istringstream is (std::string (result, written));
      std::string dummy;

      is >> dummy;
      is >> dummy;
      is >> sums.sum1;
      is >> dummy;
      is >> dummy;
      is >> sums.sum2;
    }
    catch (std::exception const & ex)
    {
      sums.sum1 = std::numeric_limits<double>::quiet_NaN();
      sums.sum2 = std::numeric_limits<double>::quiet_NaN();
    }
  }

  inline ::pnetc::type::sums::type
  run ( const ::pnetc::type::param::type& param )
  {
    const int nThread (param.nThreads);

    ::pnetc::type::sums::type *sums = new ::pnetc::type::sums::type[nThread];

    boost::thread_group threads;

    const long its (param.iterations_per_run / nThread);

    for (int tid (0); tid < nThread; ++tid)
      {
        sums[tid].param = param;
        sums[tid].param.iterations_per_run = its;

        if (tid < param.iterations_per_run % nThread)
          {
            ++sums[tid].param.iterations_per_run;
          }

        sums[tid].sum1 = sums[tid].sum2 = 0;

        threads.add_thread ( new boost::thread ( fun
                                               , boost::ref (sums[tid])
                                               , tid
                                               )
                           );

      }

    threads.join_all();

    for (int tid (1); tid < nThread; ++tid)
      {
        if (isnan(sums[tid].sum1) || isnan(sums[tid].sum2))
        {
          throw std::runtime_error ("execution failed: got NaN");
        }

        sums[0].sum1 += sums[tid].sum1;
        sums[0].sum2 += sums[tid].sum2;
      }

    sums[0].param = param;

    ::pnetc::type::sums::type ret (sums[0]);
    delete [] sums;
    return ret;
  }
} // namespace asian
