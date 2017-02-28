require 'socket'
require 'set'
require 'net/http'

class Commands
  VALUES = [
    LIGHT_ON = ->() { [0x04, 0x01] },
    LIGHT_OFF = ->() { [0x04, 0x02] },
    SATURATION = ->(val) { [0x02, val] },
    BRIGHTNESS = ->(val) { [0x03, val] },
    KELVIN = ->(val) { [0x05, val] }
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
    msg += [0x31, 0, 0, 8]
    msg += cmd
    msg += [0,0,0,zone,0]
    msg += [msg[-11..-1].reduce(&:+)&0xFF]
    
    send(msg.pack('C*'))
    
    @sequence = (@sequence + 1) % 0xFF
    
    #recv
  end
end

def get_file(cmd, value, group)
  File.expand_path(File.join(__FILE__, "../../packet_captures/sidoh_wifibox1/rgbcct_group#{group}_#{cmd}_#{value}.txt"))
end

def get_packet
  Net::HTTP.get('10.133.8.167', '/gateway_traffic/rgb_cct').split("\n").last.strip
end

milight = Milight.new

(1..4).each do |group|
  (0..0x10).each do |value|
    seen_keys = Set.new
    last_val = 0
    
    file = get_file("brightness", value, group)
    
    if File.exists?(file)
      File.read(file).split("\n").each { |x| seen_keys << x.split(' ').first }
    end
    
    puts "Processing: #{value}"
    
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
          milight.send_command(Commands::BRIGHTNESS.call(value), group)
          sleep 0.1
          print "."
        end
        
        print '*'
        
        puts "\n#{value} - #{seen_keys.length}" if last_val < seen_keys.length
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
