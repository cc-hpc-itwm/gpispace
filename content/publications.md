---
layout: page
title: Publications
permalink: /publications
---

{% assign pubs = site.data.publications | sort: "year" %}

<ol class="pub-ul pub-container">

  {% for pub in pubs reversed %}
  <li>
    <span class="author">
      <i class="fa fa-user pub-large"></i>
      {% for author in pub.authors %}
        {% if forloop.last %}
          and {{ author }}
        {% else %}
          {{ author }},
        {% endif %}
      {% endfor %}
    </span><br>
    <i class="fa fa-quote-right pub-large"></i>
    <a href="{{ pub.url }}">
      <span class="title pub-wide">{{ pub.title }}.</span>
    </a>
    <br>
    {% if pub.journal %}
    <span class="journal">{{ pub.journal }},</span>
    {% endif %}
    {% if pub.book %}
    <span class="booktitle">{{ pub.book.title }},</span>
    <span class="publisher">{{ pub.book.publisher }},</span>
    {% endif %}
    {% if pub.volume %}
    <span class="volume">vol. {{ pub.volume }},</span>
    {% endif %}
    {% if pub.pages %}
    <span class="pages">pp. {{ pub.pages }},</span>
    {% endif %}
    <span class="year">{{ pub.year }}.</span>
  </li>
  {% endfor %}

</ol>
