#!/usr/bin/ruby

require 'optparse'
require 'open-uri'
require 'tmpdir'
require 'fileutils'

# module Vbox
class TestResult
  # attr_accessor :SuccessfulTests
  attr_reader :SuccessfulTests

    SuccessfulTests = ""
    FailedTests     = ""
    Counter         = 0

    def initialize
      @SuccessfulTests = ""
      @FailedTests     = ""
      @Counter         = 1
      @out = "<?xml version=\"1.0\" encoding='ISO-8859-1' standalone='yes' ?>\n"
      @out+= "<TestRun>\n"

    end

    def parse(xmlfile)
       userdata = open(xmlfile).read
       data=userdata.gsub( /\n/, '@')
       findSuccessfull(data)
       findFailed(data)
    end

    def findSuccessfull(data)
      # tempdata = "#{data}".gsub(/.*<SuccessfulTests>(.*)<\/SuccessfulTests>.*/,'\1')
      # testid   = tempdata.gsub(/<Test id=\"(.*)\">/, '\1') + @Counter
      # puts "Test ID #{testid}\n"
      # tempdata = tempdata.gsub(/<Test id=\"/g, '<Test id="'"#{@Counter}")
      # @SuccessfulTests += "#{tempdata}"
      @SuccessfulTests += "#{data}".gsub(/.*<SuccessfulTests>(.*)<\/SuccessfulTests>.*/,'\1')

      #puts "OUT: #{@SuccessfulTests}\n"
    end

    def findFailed(data)
      @FailedTests += "#{data}".gsub(/.*<FailedTests>(.*)<\/FailedTests>.*/,'\1')
    end

    def print
      @out += "   <FailedTests>\n"
      @out += "#{@FailedTests}".gsub( /@/, "\n")
      @out += "   </FailedTests>\n"
      @out += "   <SuccessfulTests>\n"
      @out += "#{@SuccessfulTests}".gsub( /@/, "\n")
      @out += "   </SuccessfulTests>\n"
      @out += "</TestRun>\n"
      puts "#{@out}\n"
    end
  end
# end

##xmlfile = File::join("build/tests/gltests/out.xml")

xmlfile="build/tests/gwdl_out.xml"

result = TestResult.new()
result.parse(xmlfile)
result.parse("./build/tests/gwes_out.xml")
#result.parse("")
#result.parse("")

result.print()

#out=userdata.gsub( /^.*<SuccessfulTests>/, '')
# out = userdata.gsub(/.*<SuccessfulTests>(.*)<\/SuccessfulTests>.*/,'\1')
#out = userdata.gsub(/.*<SuccessfulTests>/, '')
#out = out.gsub(/<\/SuccessfulTests>.*/, '')
#out = out.gsub( /@/, "\n")

#puts "OUT: #{out}\n"
