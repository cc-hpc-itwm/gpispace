---
layout: page
title: Features
permalink: /features
---

## Separation of Concerns

To separate computation and coordination is the core idea of not only a variety of today’s task based systems but goes back in time at least into the 1980ies when David Gelernter designed Linda.
A single model subsumes different levels of parallelism and is orthogonal to concrete hardware confi gurations and to specifi c programming languages used to implement compute kernels.
That orthogonality allows for independent optimization and for generic approaches to large scale execution in a dynamic and heterogeneous environment.

## Petri Net: Managed Dependencies

Coordination means to describe dependencies between tasks.
Petri nets are a wellknown and well-understood formalism to describe concurrent systems and are used in GPI-Space as workflow description language.
Data decomposition is naturally supported by Petri nets too.
The workflow engine automatically parallelizes by maintaining a so called “front of activity” which contains all tasks that are ready to be executed at a given time.

## Large Scale Execution

The distributed runtime system of GPISpace is fed by the workfl ow engine with “activities”.
Those are tasks bundled with their input data description.
Taking into account the currently available resources and their capabilities the GPI-Space backfilling bunch scheduler dynamically assigns work to resources such that time to solution is minimized.
In case resources fail or become unavailable the runtime system reassigns work to other resources without requiring any further application support.

## Memory Driven Computing

Petri nets are powerful tools to describe data and task parallelism and to separate activation from execution.
To make large amounts of data easily accessible for the activated executions GPI-Space has introduced an application independent memory layer (IML).
This is a separate process with a dedicated API to manage memory and to provide information on data transport costs.
Data transfer within a distributed system is managed with help of the IML.
In the result all tasks are started with pointers to the input data in local memory but need not to worry about data transport at all.
Instead data transfers are scheduled and executed asynchronously by the runtime system.
At the same time the concrete physical data representation is hidden from the application.
The IML can be placed in “memory segments”, which are distributed in DRAM, HBM, NVRAM or even the BeeGFS parallel file system.

## Coupled Distinct Applications

This memory driven computing approach using the IML has the advantage that applications can easily share data.
Applications written in different languages or running on different architectures can be coupled via the IML.
Moreover, individual tasks can be legacy code and are not required to be aware of the coupling.
That means that tasks can correspond to threads in a parallel C++ program or be Matlab modules or even processes that belong to a large scale visualization program.

## Domain Specialization

GPI-Space is a general purpose system and is typically not used “bare bone” but as core of a further specialized domain specific framework.
GPI-Space supports specialization on all levels.
The Petri net workflow description is hierarchical and provides an embedded type system for user defined types and an embedded expression language that is integrated with the type system.
To specialize GPI-Space for a domain means to describe the data structures and workflow patterns once and to reuse it either from a library or in a domain specific higher level language or compiler.
