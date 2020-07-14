## Fixed
- bug related to states discovery in agents without workers in the 
  sdpa_JobStatesDiscover test was fixed. The cause was a race between 
  discover and submit in the layer causing responding immediately with a 
  discovery response with no children states.
