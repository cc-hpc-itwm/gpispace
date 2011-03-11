#!/usr/bin/env ruby
require 'xmlrpc/client'
require 'pp'

c = XMLRPC::Client.new("localhost", "/", 8080)
sdpa = c.proxy("sdpa")

pp sdpa.submit("hello")
