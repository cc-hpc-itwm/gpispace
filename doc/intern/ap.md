# AP: Scheduler Ertüchtigung

- See workplan for Arien Roussel
- See gpispace/doc/intern/performance_model/performance_model.txt (Sep 29 2015)

## Overview:

|   | Task                                 | Time             | Person
|---|--------------------------------------|------------------|----------------
| 1 | Definition of Requirements           | 2/2018 -  3/2018 | Adrien + Team
| 2 | Definition of Datastructures 1       | 3/2018 -  9/2019 | Adrien
| 3 | Standalone Prototype                 | 9/2018 -  3/2019 | Adrien
| 4 | Definition of Datastructures 2       |11/2018 -  5/2019 | Adrien
| 5 | Roadmap: Integration into GPI-Space  |11/2018 -  5/2019 | Adrien + Team
| 6 | Implementation in Branch             | 4/2019 - 10/2019 | ?
| 7 | Use (of features) in App(s)          | 9/2019 -  1/2020 | ?
| 8 | Merge and adaptation                 | 1/2020 -  4/2020 | ?

## Dependencies:

```
digraph
{
1    2    3    4    5    6    7    8

1 -> 2 -> 3
     2 ->      4
     2 ->           5 -> 6 -> 7 -> 8
               4 -> 5 -> 4
          3      -> 5 -> 3
}
```

## Gantt:

```
   2018       2019        2020
           111         111
   234567890121234567890121234
1  ##
2   #######
3         #######
4           #######
5           #######
6                #######
7                     #####
8                         ####
```

## Details:

### Task 1: Definition of Requirements

- Goal:
Define a detailled list of requirements for the scheduler, that includes functional (e.g. quality of schedule under given performance model and with co-scheduling) and non-functional (e.g. running times, cancellation) requirements. The requirements define the working context for the scheduler, e.g. granularities, frequencies, system sizes (e.g. 10000 cores), task properties (e.g. GPU, mem, GPI), system dynamics (e.g. add/remove nodes), the required precision (e.g. within a factor of two of the optimal schedule) and the required scheduler performance, e.g. milli overhead. Define benchmark scenarios.

- Method:
  * 2 Workshops, 1 day each.
  * In between: Write (Semi-)formal definition.

- Results:
  * Detailled list of requirements and wanted properties, functional and non-functional in form of a document with (semi-)formal definition.
  * Definition of black box benchmark scenarios.

- Risks:
  * Requirements are overlooked (medium), Solution: Full restart.

### Task 2: Definition of Datastructures 1

- Goal: Define the external API of a set of datastructures that are suitable to fulfill the requirements (as much as possible) defined in Task 1. The external API is used by the runtime system and the worker manager of GPI-Space. The definition respects the current software architecture where possible and gives hints of how to change it otherwise. It is proven that all required information can be submitted to the scheduler. It is plausible that the requirements can be fulfilled. Usage for some common scenarios is documented in form of C++ test code.

- Method: Read. Talk. Think. Write.

- Results:
  * Paper with API definition, descriptions and proofs.
  * C++ Header.

- Risks:
  * Too many requirements are not met (medium), Solution: Go back to Task 1.

### Task 3: Standalone Prototype

- Goal: Implement a standalone prototype for the data structures defined in Task 2. The prototype simulates the GPI-Space context based on application scenarios. Minor API parts might be missing. The algorithmic complexity is correct for the most time consuming parts. Testsuite succeeds.

- Method: Close door. Type.

- Results:
  * C++ Code.

- Risks:
  * Implementation impossible/too hard (low). Solution: Go back to Task 2.
  * Lack of implemention skills (low). Solution: Teach, support.

### Task 4: Definition of Datastructures 2

- Goal: Parallel to the prototype implementation redefine the datastructures where needed. Extend the testsuite and proofs. Define internal API that is used by the scheduler components. Define unit tests. Define white box benchmark scenarios.

- Method: Think. Try. Adjust.

- Results:
  * Paper with API definition, descriptions, proofs and benchmark results.
  * C++ Header.

- Risks:
  * Design flaws are uncovered (medium). Solution: Go back to Task 2.

### Task 5: Roadmap: Integration into GPI-Space

- Goal: Define a roadmap how to integrate the defined scheduler data structures into GPI-Space. The expectation is that the current architecture of GPI-Space is not directly applicable to the new data structures. The roadmap contains steps and their order that allow to restructure GPI-Space in order to make efficient use of the new data structures.

- Method: Analysis and developer meetings.

- Results:
  * Workplan.

- Risks:
  * GPI-Space does not fit with the new data structures (low), Solution: Go back to Task 2.

### Task 6: Implementation in Branch

- Goal: The new data structures are implemented in a branch of the GPI-Space source tree. That includes changes to the GPI-Space architecture, the new data structures, unit tests and system tests that are included in the GPI-Space test suite. Small parts of the adapatation might be not as efficient as possible. The algorithmic complexity is correct for the most time consuming parts.

- Method: Close door. Type.

- Results:
  * A branch of GPI-Space that uses the new data structures.

- Risks:
  * The adaptation uncovers dependencies not forseen in the roadmap (medium). Solution: Go back to Task 5.
  * The implementation uncovers design flaws (medium). Solution: Go back to Task 2.

### Task 7: Use (of features) in App(s)

- Goal: Based on the branch implemented in Task 6 the new scheduler is used in GPI-Space applications like Splotch, RTM or DLPS. New features are effectively used and advantages are demonstrated.

- Method: Close door. Type.

- Results:
  * Two or more applications that use the new scheduler.
  * Ported applications make effective use of new features. (e.g. performance model)
  * Guide to port applications.

- Risks:
  * Porting impossible (very low). Solution: Go back to Task 2.
  * Application looses performance. (low). Solution: Go back to Task 2.

### Task 8: Merge and adaptation

- Goal: The new scheduler is merged into the main branch of GPI-Space and all supported applications are ported.

- Method: Read. Talk. Think. Close door. Type.

- Results:
  * The new scheduler data structures are merged into the master branch of GPI-Space.
  * All supported applications are ported.
  * Ported applications make effective use of new features.

- Risks:
  * Some applications can't be ported (very low). Solution: Go back to Task 2.
  * The GPI-Space master has diverged too much (low). Solution: Go back to Task 5.
