## Added
### Modify worker environment variables
It is now possible to specify the environment variables to be set when
starting a runtime system's workers, rather than having a predefined,
empty, environment.

By default the environment still is completely empty, and no variables
other than the ones referenced in the options will be set in the
remote process. The following options have been added:

- `--worker-env-set-variable` to explicitly set the value of a
  variable on the command line
- `--worker-env-copy-file path` to set all variables in the given
  files, where every line is a key-value pair in `env` style (`key=value`)
- `--worker-env-copy-variable name` and `--worker-env-copy-current` to
  copy variables set in the process starting up by allow-list or
  blanket copy respectively

## Removed
- `$install/lib` and `$install/libexec/gspc` is no longer added to
  `LD_LIBRARY_PATH` of worker automatically. Users may use
  `--worker-env-set-variable` to reconstruct this behavior, but proper
  `rpath`s are suggested instead.
