---
layout: post
title: "DMV-ÖMG Minisymposium 28 Talk: Massively Parallel Workflows with GPI-Space"
teaser_image: assets/img/teaser/dmv-oemg-2021.png
---

We gave a talk presenting GPI-Space and its components at the [DMV-ÖMG Annual Conference 2021](https://www.uni-passau.de/en/dmv-oemg-jahrestagung-2021/home/) in the *Minisymposium 28: Massively Parallel Methods in Geometry and Applications*.

Have a look at the abstract and slides below:

## Massively Parallel Workflows with GPI-Space

---

*B. Lörwald*<sup>1</sup>, *R. Machado*<sup>1</sup>, *M. Rahn*<sup>1</sup>, *T. Rotaru*<sup>1</sup>, *M. Zeyen*<sup>1</sup>

<sup>1</sup> Fraunhofer ITWM

---

<br/>
[<i class="fa fa-download"></i> Slides]({{ "assets/docs/dmv-oemg-21-09-30.pdf" | relative_url }})

### Abstract:

Conventional programming models make it hard to leverage the available computing power of distributed high performance systems to its fullest without expert knowledge.
Handling the aspects of efficient storage management and fault-tolerant workflow executions can exceed the capabilities of normal users.

In this talk, we are presenting GPI-Space and its components, a task-based workflow management system for massively parallel applications, developed at the Fraunhofer ITWM.
GPI-Space is designed to automatically coordinate scalable, parallel executions in large, complex environments.
It allows domain developers to focus on their domain-specific workflows, while relying on GPI-Space to take care of general aspects related to scheduling, distributed memory management, and fault-tolerant task execution.
