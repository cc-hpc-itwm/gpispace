---
layout: home
title: Home
list_title: Latest News
permalink: /
---

<div class="home-grid-container">

  <div class="home-grid-item home-grid-main">
    <h1>GPI-Space</h1>
    <h2>Memory Driven Computing</h2>
    <div>
      <img src="{{ "assets/img/GPISpace_arch.svg" | relative_url }}" alt="GPI-Space Architecture" width="100%"/>
    </div>
    <br/>
    <p style="text-align:left">
      GPI-Space is a task-based workflow management system for parallel applications.
      It allows the developers to build domain-specific workflows using their own parallelization patterns, data management policies and I/O routines, while relying on the runtime system for the workflow management.
      The GPI-Space ecosystem "auto-manages" the application runs with dynamic scheduling, in-built distributed memory transfer and distributed task execution.
    </p>
    <a href="{{ "/installation" | relative_url }}" class="home-grid-main-button">
      <strong>First Steps</strong>
    </a>
  </div>

  <div class="home-grid-item home-grid-card-1">
    <h3>Features</h3>
    <ul style="text-align:left">
      <li>Separation of Concerns</li>
      <li>Petri Net: Managed Dependencies</li>
      <li>Large Scale Execution</li>
      <li>Memory Driven Computing</li>
      <li>Coupled Distinct Applications</li>
      <li>Domain Specialization</li>
    </ul>
    <a href="{{ "/features" | relative_url }}" class="home-grid-card-button">
      <strong>Learn More</strong>
    </a>
  </div>

  <div class="home-grid-item home-grid-card-2">
    <h3>Open Source</h3>
    <p style="text-align:left">
      GPI-Space is an open source software library and freely available to application developers and researchers under the GPLv3 license.
      For commercial users we offer a commercial license and support.
    </p>
    <a href="{{ "/contact" | relative_url }}" class="home-grid-card-button">
      <strong>Contact Us</strong>
    </a>
  </div>

</div>
