#!/usr/bin/env ruby

require 'fileutils'

1000000.times {
  th = []
  (8080..8089).each {|p|
    th << Thread.new(p){|port|
      (0..9).each{|i|
        (0..9).each{|j|
          FileUtils.cp("tmp/localhost:#{port}/#{i}/#{j}", "/dev/null")
        }
      }
#      FileUtils.cp("tmp/localhost:#{port}/index.html", "/dev/null")
#      FileUtils.cp("tmp/localhost:#{port}/data", "/dev/null")
#      FileUtils.cp("tmp/localhost:#{port}/data.bin", "/dev/null")
    }
  }
  th.each{|t| t.join}
}
