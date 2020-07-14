## Added
### RIF strategy 'local'
Developers testing on machines that support neither `ssh` nor `pbsdsh`
may now at least validate their application on the `local` node by
specifying `--rif-strategy=local`. The strategy expects the only node
given to be `hostname()`.
## Fixed
- Properly clean up processes when using rif strategy `pbsdsh`
