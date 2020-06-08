#include <rif/strategy/srun.hpp>

#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <util-generic/blocked.hpp>
#include <util-generic/getenv.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace fhg
{
  namespace util
  {
    namespace syscall
    {
// vvvv src/util-generic/syscall.hpp vvvv
      //! `close (fd)`, but ignore EBADF. Still signals all other
      //! errors.
      void best_effort_close (int fd);

      union pipe_fds
      {
        int both[2];
        struct
        {
          int read;
          int write;
        };
      };
      pipe_fds pipe();
      pipe_fds pipe (int flags);

      namespace abort_on_error
      {
        void close (int fd);
        void execve (const char* filename, char* const argv[], char* const envp[]);
        /*size_t*/ void read (int fd, void* buf, size_t count);

        //! `close (fd)`, but ignore EBADF. Still signals all other
        //! errors.
        void best_effort_close (int fd);
      }
// ^^^^ src/util-generic/syscall.hpp ^^^^

// vvvv src/util-generic/syscall.cpp vvvv
      void best_effort_close (int fd)
      {
        auto const rc (::close (fd));
        if (rc == -1 && errno != EBADF)
        {
          throw boost::system::system_error
            (boost::system::error_code (errno, boost::system::system_category()));
        }
      }

      pipe_fds pipe()
      {
        pipe_fds pipefd;
        pipe (pipefd.both);
        return pipefd;
      }
      pipe_fds pipe (int flags)
      {
        pipe_fds pipefd;
        pipe (pipefd.both, flags);
        return pipefd;
      }

      namespace abort_on_error
      {
        void close (int fd)
        {
          if (::close (fd) == -1)
          {
            std::abort();
          }
        }
        void execve (const char* filename, char* const argv[], char* const envp[])
        {
          ::execve (filename, argv, envp);

          // execve either does not return, or returns negative,
          // which indicates an error
          std::abort();
        }
        /*size_t*/ void read (int fd, void* buf, size_t count)
        {
          if (::read (fd, buf, count) == -1)
          {
            std::abort();
          }
        }

        void best_effort_close (int fd)
        {
          if (::close (fd) == -1 && errno != EBADF)
          {
            std::abort();
          }
        }
      }
// ^^^^ src/util-generic/syscall.cpp ^^^^
    }
  }

  namespace rif
  {
    namespace strategy
    {
      namespace srun
      {
        namespace
        {
          namespace option
          {
            namespace validator = util::boost::program_options;

            constexpr char const* const block_size {"sruns-at-once"};
            constexpr char const* const block_size_description
              {"how many SRUN connections to establish at once"};
            using block_size_type = std::size_t;
            using block_size_validator
              = validator::positive_integral<std::size_t>;
            constexpr block_size_type const block_size_default = 64;

            constexpr char const* const job_id {"job-id"};
            constexpr char const* const job_id_description
              { "Slurm job ID to allocate in, defaults to $SLURM_JOBID "
                "or $SLURM_JOB_ID if set"
              };
            using job_id_type = std::string;
            using job_id_validator = validator::nonempty_string;

            boost::program_options::typed_value<job_id_validator>*
              make_job_id_option()
            {
              auto value (boost::program_options::value<job_id_validator>());

              auto const slurm_jobid (util::getenv ("SLURM_JOBID"));
              auto const slurm_job_id (util::getenv ("SLURM_JOB_ID"));
              if ( slurm_jobid && slurm_job_id
                 && std::string (*slurm_job_id) != std::string (*slurm_jobid)
                 )
              {
                // don't default: inconsistent environment, require
                // user to specify.
              }
              else if (slurm_job_id || slurm_jobid)
              {
                value->default_value ( slurm_jobid
                                     ? std::string (*slurm_jobid)
                                     : std::string (*slurm_job_id)
                                     );
              }
              else
              {
                // don't default: require user to specify.
              }

              return value->required();
            }
          }

          //! \todo use util::boost::program_options::generic
#define EXTRACT_PARAMETERS(parameters_)                                 \
          boost::program_options::options_description options;          \
          options.add_options()                                         \
            ( option::block_size                                        \
            , boost::program_options::value<option::block_size_validator>() \
              ->default_value (option::block_size_default)              \
            , option::block_size_description                            \
            )                                                           \
            ( option::job_id                                            \
            , option::make_job_id_option()                              \
            , option::job_id_description                                \
            )                                                           \
            ;                                                           \
                                                                        \
          boost::program_options::variables_map vm;                     \
          boost::program_options::store                                 \
            ( boost::program_options::command_line_parser (parameters)  \
              .options (options).run()                                  \
            , vm                                                        \
            );                                                          \
                                                                        \
          boost::program_options::notify (vm);                          \
                                                                        \
          boost::optional<option::block_size_type> const block_size     \
            ( vm.count (option::block_size)                             \
            ? boost::optional<option::block_size_type>                  \
                (vm.at (option::block_size).as<option::block_size_validator>()) \
            : boost::none                                               \
            );                                                          \
                                                                        \
          option::job_id_type const job_id                              \
            (vm.at (option::job_id).as<option::job_id_validator>())

          //! The maximum number of digits needed to print the given
          //! value of type \c Integral in base 10.
          //! \note Is not equivalent to numeric_limits<T>::digits10:
          //! that's the number of digits guaranteed to be
          //! representable when parsed, and thus usually one less
          //! than this.
          //! \note Does not include a trailing null byte.
          template<typename Integral>
            constexpr int digits10_full
              (Integral n = std::numeric_limits<Integral>::max())
          {
            return n ? digits10_full (n / Integral (10)) + 1 : 0;
          };

          //! A format string conversion specifier that represents the
          //! given \c T.
          template<typename T>
            constexpr char const* printf_format();
          template<>
            constexpr char const* printf_format<int>()
          {
            return "%d";
          }

          void do_srun ( std::string const& hostname
                       , std::string const& job_id
                       , boost::optional<unsigned short> const& port
                       , std::string const& register_host
                       , unsigned short register_port
                       , boost::filesystem::path const& binary
                       )
          {
            std::vector<std::size_t> argv_offsets;
            std::vector<char> argv_buffer;

            auto const add_argument
              ( [&] (std::string const& argument)
                {
                  argv_offsets.emplace_back (argv_buffer.size());
                  argv_buffer.insert
                    (argv_buffer.end(), argument.begin(), argument.end());
                  argv_buffer.emplace_back ('\0');
                }
              );

            add_argument ("srun");

            // only show errors
            add_argument ("--quiet");

            // don't wait if for some reason we don't get resources
            add_argument ("--immediate");
            // don't consume CPUs: not in sync with how many workers
            // we start anyway so more misleading than helping
            add_argument ("--oversubscribe");

            // be sure we are self-contained
            add_argument ("--export=NONE");

            // use specific host
            add_argument ("--nodelist=" + hostname);
            // with exactly one instance
            add_argument ("--ntasks=1");

            // don't require environment and allow override
            add_argument ("--jobid=" + job_id);

            add_argument (binary.string());

            if (port)
            {
              add_argument ("--port=" + std::to_string (*port));
            }
            add_argument ("--register-host=" + register_host);
            add_argument ("--register-port=" + std::to_string (register_port));
            add_argument ("--register-key=" + hostname);

            add_argument ("--dont-fork");

            // We need to avoid any exceptions in the child process
            // before `execve()`. Thus,

            // - prepare the pid argument by pre-allocating enough
            //   space and remembering the position where to write the
            //   pid.
            argv_offsets.emplace_back (argv_buffer.size());
            std::string const override_pid_arg_prefix ("--override-pid=");
            argv_buffer.insert ( argv_buffer.end()
                               , override_pid_arg_prefix.begin()
                               , override_pid_arg_prefix.end()
                               );
            auto const pid_arg_pos (argv_buffer.size());
            auto const pid_arg_size (digits10_full<pid_t>() + sizeof ('\0'));
            argv_buffer.insert (argv_buffer.end(), pid_arg_size, '\0');
            auto const pid_arg_ptr (argv_buffer.data() + pid_arg_pos);

            // - prepare argv and envp
            std::vector<char*> argv;
            for (auto const& offset : argv_offsets)
            {
              argv.emplace_back (argv_buffer.data() + offset);
            }
            argv.emplace_back (nullptr);
            std::vector<char*> const envp {nullptr};

            // - determine constant that may throw
            auto const maximum_open_files
              (util::syscall::sysconf (_SC_OPEN_MAX));


            auto const pipe_fds (util::syscall::pipe());

            pid_t pid (util::syscall::fork());

            if (pid)
            {
              util::syscall::close (pipe_fds.read);
              util::syscall::write (pipe_fds.write, &pid, sizeof (pid));
              util::syscall::close (pipe_fds.write);

              // We can't control what `argv[0]` does, so there is
              // nothing really we can do to ensure it actually
              // started, so there is nothing to wait for here as we
              // only want to `waitpid()` when we want to tear it
              // down. We could poll for a second to ensure it doesn't
              // immediately terminate, but that's just a race as
              // well.
              // \todo This implies that we would have some timeout in
              // waiting for registrations, which we don't. If
              // anything crashes, the user can only determine that by
              // seeing an error output and killing the job. Poll on
              // some Slurm binary that another job step was added?
              // \todo Alternatively, add dependency to libslurm and
              // use their C API...
            }
            else
            {
              util::syscall::abort_on_error::close (pipe_fds.write);
              util::syscall::abort_on_error::read
                (pipe_fds.read, &pid, sizeof (pid));
              util::syscall::abort_on_error::close (pipe_fds.read);

              for (int fd (0); fd < maximum_open_files; ++fd)
              {
                // Keep output alive to -- even if we --quiet -- see
                // errors and warnings, if there are any.
                if (fd == STDERR_FILENO || fd == STDOUT_FILENO)
                {
                  continue;
                }
                util::syscall::abort_on_error::best_effort_close (fd);
              }

              if ( std::snprintf ( pid_arg_ptr, pid_arg_size
                                 , printf_format<pid_t>()
                                 , pid
                                 ) < 0
                 )
              {
                std::abort();
              }

              // \todo If we daemonize here bootstrap and teardown in
              // different processes would work, assuming that
              // bootstrap was made from outside a job, or that job
              // step lives until teardown.

              util::syscall::abort_on_error::execve
                (argv[0], argv.data(), envp.data());
            }
          }
        }

        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& all_hostnames
                    , boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , boost::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream&
                    )
        {
          EXTRACT_PARAMETERS (parameters);

          return util::blocked_async<std::string>
            ( all_hostnames
            , block_size
            , [] (std::string const& hostname) { return hostname; }
            , [&] (std::string const& hostname)
              {
                do_srun ( hostname
                        , job_id
                        , port
                        , register_host
                        , register_port
                        , binary
                        );
              }
            ).second;
        }

        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
            ( std::unordered_map<std::string, fhg::rif::entry_point> const& all_entry_points
            , std::vector<std::string> const& parameters
            )
        {
          EXTRACT_PARAMETERS (parameters);

          return util::blocked_async<std::string>
            ( all_entry_points
            , block_size
            , [] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                return entry_point.first;
              }
            , [&] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                util::system_with_blocked_SIGCHLD
                  ( str ( boost::format ("/bin/kill -TERM %1%")
                        % entry_point.second.pid
                        )
                  );
              }
            );
        }
      }
    }
  }
}
