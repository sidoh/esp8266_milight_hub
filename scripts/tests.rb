require 'pp'
require 'set'

CAPTURES_DIR = File.expand_path(File.join(__FILE__, "../../packet_captures"))

def first_byte_determines_next(capture, count)
  capture
    .group_by { |x| x[0] }
    .map { |_, x| x.map { |packet| packet[1..count] } }
    .map(&:uniq)
    .map(&:length)
    .all? { |x| x == 1 }
end

def parse_capture(filename)
  File
    .read(filename)
    .split("\n")
    .map { |x| x.split(' ').map { |b| b.to_i(16) } }
end

Dir.glob("#{CAPTURES_DIR}/**.txt").each do |filename|
  puts File.basename(filename)
  capture = parse_capture(filename)
  
  pp first_byte_determines_next(capture, 3)
  
  # a = capture
  #   .group_by { |x| x[0] }
  #   .sort
  #   .map { |_, packets| [packets.first[0], packets.first[1]] }
  # mappings = Hash[a]
  # perms = []
  # current_perm = []
  # elem = a.first
  # current_perm = [elem[0]]
  # visited = Set.new([elem[0]])
  # perm_visited = Set.new([elem[0]])
  # 
  # while visited.length < mappings.length
  #   new_elem = nil
  #   
  #   puts visited.inspect
  #   
  #   if !mappings.include?(elem[1]) || perm_visited.include?(mappings[elem[1]])
  #     new_elem = (Set.new(mappings.keys) - visited).first
  #     perms << current_perm
  #     current_perm = []
  #     perm_visited = Set.new
  #   else
  #     new_elem = [elem[1], mappings[elem[1]]]
  #   end
  #   
  #   visited << new_elem[0]
  #   perm_visited << new_elem[0]
  #   elem = new_elem
  # end
  # 
  # perms << current_perm
  # 
  # pp perms
  
  a = capture
   .group_by { |x| x[0] }
   .sort
   .map { |_, packets| [packets.first[0], packets.first[1]] }
   .sort { |x,y| x[1] <=> y[1] }
  a = a
   .each_with_index.map do |x, i|
     [x[1],x[0],if a[i-1][1] == x[1]-1
       (x[0]-a[i-1][0])
     else
       nil
     end]
   end
   .to_a
  #  # .group_by { |x| x[1] }
  #  # .sort
  #  # .to_a
  
  pp a
  
  # a = capture
  #   .group_by { |x| x[0] }
  #   .map { |_, packets| packets.first }
  #   .sort #{ |x, y| x[1] <=> y[1] }
  #   .map { |x| x }
  #   .to_a
  #   
end

# on1 = parse_capture("#{CAPTURES_DIR}/rgbwcct_group1_on.txt")
# on2 = parse_capture("#{CAPTURES_DIR}/rgbwcct_group2_on.txt")
# 
# on1_g = on1.group_by { |x| x[0] }
# on2_g = on2.group_by { |x| x[0] }
# 
# a = (on1 + on2)
#   .map { |x| x[0] }
#   .sort
#   .uniq
#   .map do |key|
#     {key: key, vals: {on1: (on1_g[key]||[]).map { |x| x[1] }, on2: (on2_g[key]||[]).map { |x| x[1] } }}
#   end
#   .to_a
# pp a