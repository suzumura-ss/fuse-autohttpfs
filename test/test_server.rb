#!/usr/bin/env ruby

require 'rubygems'
require 'webrick'
require 'date'
require 'json'


class TestServlet < WEBrick::HTTPServlet::AbstractServlet
  def initialize(server, *opt)
    @opt = opt[0] || {}
    @mtime = (@opt["mtime"] || Time.now).iso8601
    dir = @opt["dir"] || {}
    @dirs = dir["entries"] || %w{0 1 2 3 4}
    @mode_dir = dir["mode"] || "drwx------"
    reg = @opt["reg"] || {}
    @regs = reg["regs"] || %w{a b c d e}
    @mode_reg = reg["mode"] || "-rwx------"
    build_entries
  end

  def do_GET(req, res)
    res.body = body if do_HEAD(req, res)
  end

  def do_HEAD(req, res)
    name = File.basename(req.path)
    res["Content-Type"] = 'text/json'
    res["Content-Length"] = @size_reg
    if name=~%r{^(\d+|/)$}
      res["X-FileStat-Json"] = { name=>stat_dir }.to_json
    else
      res["X-FileStat-Json"] = { name=>stat_reg }.to_json
    end
    true
  end

  def mtime=(t)
    @mtime = t.iso8601
    build_entries
  end

  def stat_dir=(m)
    @stat_dir = m
    build_entries
  end

  def stat_reg=(m)
    @stat_reg = m
    build_entries
  end

private
  def stat_dir
    {"mode"=>@mode_dir, "size"=>@size_dir, "mtime"=>@mtime}
  end

  def stat_reg
    {"mode"=>@mode_reg, "size"=>@size_reg,  "mtime"=>@mtime}
  end

  def body
    @entries.to_json + "\n"
  end

  def build_entries
    @size_dir = 4096
    @size_reg = 0
    while @size_reg!=body.length
      @size_reg = body.length
      _build_entries
    end
  end

  def _build_entries
    @entries = Hash.new
    @dirs.map{|n| @entries[n] = stat_dir }
    @regs.map{|n| @entries[n] = stat_reg }
  end
end


if $0==__FILE__
  STDERR.sync = true
  opt = ARGV[1] ? YAML.load_file(ARGV[1]): {}
  s = WEBrick::HTTPServer.new(:Port=>(ARGV[0]||8000).to_i)
  s.mount("/", TestServlet, opt)
  trap(:INT){ s.shutdown }
  s.start
end
