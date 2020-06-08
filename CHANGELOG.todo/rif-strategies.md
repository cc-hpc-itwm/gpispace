## Added
### Bootstrap via srun
In addition to the existing runtime system bootstrap strategies `ssh`
and `pbsdsh`, support for Slurm's `srun` was added.
## Fixed
- Properly clean up processes when using rif strategy `pbsdsh`
