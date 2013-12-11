
#include <iostream>
#include <string>
#include <stdexcept>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <writer.hpp>

int main (int argc, char** argv)
{
  namespace po = boost::program_options;

  typedef boost::mt19937 gen_type;
  typedef boost::normal_distribution<> dist_type;

  unsigned long seed (31415926);
  unsigned long number (1000);
  double mean (0.0);
  double sigma (1.0);
  std::size_t size_buf (1UL << 22);
  std::size_t num_buf (1UL << 3);
  std::string output ("-");

  po::options_description desc ("General");

  desc.add_options()
    ( "help,h", "this message")
    ( "output,o"
    , po::value<std::string> (&output)->default_value (output)
    , "output file, - for stdout"
    )
    ( "seed,s"
    , po::value<unsigned long> (&seed)->default_value (seed)
    , "seed for mersenne twister"
    )
    ( "number,n"
    , po::value<unsigned long> (&number)->default_value (number)
    , "how many values to generate"
    )
    ( "mean,m"
    , po::value<double> (&mean)->default_value (mean)
    , "mean"
    )
    ( "sigma,g"
    , po::value<double> (&sigma)->default_value (sigma)
    , "sigma"
    )
    ( "size_buf,b"
    , po::value<std::size_t> (&size_buf)->default_value (size_buf)
    , "size_buf"
    )
    ( "num_buf,k"
    , po::value<std::size_t> (&num_buf)->default_value (num_buf)
    , "size_buf"
    )
    ;

  po::variables_map vm;

  try
  {
    po::store (po::parse_command_line(argc, argv, desc), vm);
    po::notify (vm);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("help"))
  {
    std::cout << argv[0] << std::endl << desc << std::endl;

    return EXIT_SUCCESS;
  }

  const std::size_t elem_per_buf (size_buf / sizeof (double));

  gen_type gen (seed);
  dist_type dist (mean, sigma);
  double* buf (new double[elem_per_buf * num_buf]);

  boost::variate_generator<gen_type, dist_type> generator (gen, dist);

  typedef writer<double> writer_type;
  typedef writer_type::queue_type queue_type;
  typedef writer_type::buffer_type buffer_type;

  queue_type queue_empty;
  queue_type queue_full;

  FILE* file = stdout;

  if (output != "-")
  {
    file = fopen (output.c_str(), "w");

    if (!file)
    {
      throw std::runtime_error ("could not open " + output);
    }
  }

  boost::thread thread_writer (writer_type (queue_empty, queue_full, file));

  double* begin (buf);
  double* end (begin + elem_per_buf * num_buf);

  while (begin < end)
    {
      const std::size_t count
        (std::min (elem_per_buf, static_cast<std::size_t>(end - begin)));

      queue_empty.put (buffer_type (begin, count));

      begin += count;
    }

  double sum (0.0);
  double sqsum (0.0);

  for (unsigned long i (0); i < number;)
    {
      buffer_type buf (queue_empty.get());

      double* val (buf.begin());
      std::size_t end (std::min (number, i + buf.size()));
      buf.count() = 0;

      for (unsigned long j (i); j < end; ++j, ++i, ++val, ++buf.count())
        {
          const double r (generator());

          sum += r;
          sqsum += r*r;
          *val = r;
        }

      queue_full.put (buf);
    }

  queue_full.put (buffer_type (0, 0));

  thread_writer.join();

  if (output != "-")
    {
      fclose (file);
    }

  delete[] buf;

  std::cerr << "sum = " << sum << std::endl;
  std::cerr << "sqsum = " << sqsum << std::endl;
  std::cerr << "number = " << number << std::endl;
  std::cerr << "mean = " << sum / number << std::endl;
  std::cerr << "sigma = " << (sqsum - sum * sum / number) / number << std::endl;
}
