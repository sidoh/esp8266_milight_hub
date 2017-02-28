require 'socket'
require 'set'
require 'net/http'

STD_COMMAND_PREFIX = [ 0x31, 0x00, 0x00, 0x08 ]
STD_COMMAND_SUFFIX = [ 0x00, 0x00, 0x00 ]

class Commands
  def self.std_command(*cmd)
    ->() { STD_COMMAND_PREFIX + cmd + STD_COMMAND_SUFFIX }
  end

  def self.arg_command(prefix)
    ->(val) { STD_COMMAND_PREFIX + [prefix, val] + STD_COMMAND_SUFFIX }
  end

  VALUES = [
    LIGHT_ON = std_command(0x04, 0x01),
    LIGHT_OFF = std_command(0x04, 0x02),
    SATURATION = arg_command(0x02),
    BRIGHTNESS = arg_command(0x03),
    KELVIN = arg_command(0x05),
    WHITE_ON = std_command(0x05, 0x64),
    LINK = ->() { [ 0x3D, 0x00, 0x00, 0x08, 0x00, 0x00 ] + STD_COMMAND_SUFFIX },
    UNLINK = ->() { [ 0x3D, 0x00, 0x00, 0x08, 0x00, 0x00 ] + STD_COMMAND_SUFFIX },
    COLOR = ->(value) { STD_COMMAND_PREFIX + [0x01] + ([value]*4) }
  ]
end

class Milight
  ADDR = ['<broadcast>', 5987]
  
  attr_reader :socket
  
  def initialize
    @socket = UDPSocket.new
    socket.setsockopt(Socket::SOL_SOCKET, Socket::SO_BROADCAST, true)
    @sequence = 0
  end
  
  def hex_to_bytes(s)
    s.strip.split(' ').map { |x| x.to_i(16) }.pack('C*')
  end

  def send(msg)
    socket.send(msg, 0, ADDR[0], ADDR[1])
  end

  def recv
    socket.recvfrom(1000)[0]
  end

  def start_session
    tries = 5
    begin
      Timeout.timeout(5) do
        send(hex_to_bytes("20 00 00 00 16 02 62 3A D5 ED A3 01 AE 08 2D 46 61 41 A7 F6 DC AF D3 E6 00 00 1E"))
        msg = recv
        @session = msg[-3..-2].bytes
      end
    rescue Exception => e
      puts "Error: #{e}"
      retry if (tries -= 1) > 0
    end
  end

  def send_command(cmd, zone = 0)
    start_session if !@session
    
    msg = [0x80, 0, 0, 0, 0x11, @session[0], @session[1], 0, @sequence, 0]
    msg += cmd
    msg += [zone,0]
    msg += [msg[-11..-1].reduce(&:+)&0xFF]
    
    send(msg.pack('C*'))
    
    @sequence = (@sequence + 1) % 0xFF
    
    #recv
  end
end

def get_file(cmd, value, group)
  name = "../../packet_captures/sidoh_wifibox1/rgbcct_group#{group}_#{cmd}#{value.nil? ? "" : "_#{value}"}.txt"
  File.expand_path(File.join(__FILE__, name))
end

def get_packet
  Net::HTTP.get('10.133.8.167', '/gateway_traffic/rgb_cct').split("\n").last.strip
end

milight = Milight.new

(1..4).each do |group|
  (0..0xFF).each do |value|
    seen_keys = Set.new
    last_val = 0
    
    file = get_file("color", value, group)
    
    if File.exists?(file)
      File.read(file).split("\n").each { |x| seen_keys << x.split(' ').first }
    end
    
    puts "Processing: #{file}"
    
    File.open(file, 'a') do |f|
      while seen_keys.size < 255
        t = Thread.new do 
          packet = get_packet
          key = packet.split(' ')[0]
          seen_keys << key
          f.write "#{packet}\n"
          f.flush
        end
        
        while %w(sleep run).include?(t.status)
          milight.send_command(Commands::COLOR.call(value), group)
          sleep 0.1
          print "."
        end
        
        print '*'
        
        puts "\n#{file} - #{seen_keys.length}" if last_val < seen_keys.length
        last_val = seen_keys.length
      end
    end
  end
end

# 10000.times do
#   milight.send_command(, 1).inspect
#   print "."
#   sleep 0.1
# end
