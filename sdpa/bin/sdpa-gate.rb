#!/usr/bin/env ruby
require 'xmlrpc/server'
require 'open3'

s = XMLRPC::Server.new(8080)

def submit(desc)
  Open3.popen3("sdpac submit /dev/stdin") {|stdin, stdout, stderr|
    stdin.write(desc)
    o = stdout.read
    e = stderr.read
    if e
      raise XMLRPC::FaultException.new(1, "sdpac submit failed: #{e}")
    else
      o
    end
  }
end

s.add_introspection
s.add_multicall
s.add_handler("sdpa.submit") do |desc|
  submit(desc)
end

s.serve()
