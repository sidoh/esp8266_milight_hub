require_relative 'helpers'

COLOR_S2_BASES = [0x5A, 0x22, 0x30, 0x11]
KELVIN_S2_BASES = COLOR_S2_BASES.map { |x| x + 0x80 }
SATURATION_BRIGHTNESS_S2_BASES = [0x9A, 0x62, 0x70, 0x51]

# saturation bases + 0x80
COMMAND_BASES = [0x1A, 0xE2, 0xF0, 0xD1]

GROUP_BASES = [0xAF, 0x04, 0xDD, 0x07]

OFFSETTING_S2_FN = ->(bases, s, e) do
  ->(b0) do
    value = bases[ (b0 % 4) ]
    if b0 >= s && b0 <= e
      value = (value + 0x80) % 0x100
    end
    
    [value]
  end
end

extra_args = {
  kelvin: {
    increments: 2,
    s1_range: [0x0C],
    s2_range: OFFSETTING_S2_FN.call(KELVIN_S2_BASES, 0x14, 0x93)
  },
  color: {
    s1_range: [0x15],
    s2_range: OFFSETTING_S2_FN.call(COLOR_S2_BASES, 0x14, 0x93)
  },
  brightness: {
    s1_range: [0x0F],
    s2_range: OFFSETTING_S2_FN.call(SATURATION_BRIGHTNESS_S2_BASES, 0x54, 0xD3)
  },
  saturation: {
    invert_key: true,
    s1_range: [0x0E],
    s2_range: OFFSETTING_S2_FN.call(SATURATION_BRIGHTNESS_S2_BASES, 0x54, 0xD3)
  },
  on_seq: {
    s1_range: [0x00],
  },
  group: {
    s1_range: [0x00],
    s2_range: OFFSETTING_S2_FN.call(GROUP_BASES, 0x54, 0xD3)
  },
  checksum: {
    s1_range: ->(x) do
      lsn = (((x & 0xF)^2) + 7) % 0x10
      
      (0..0xF).map { |msn| (msn << 4) + lsn }
    end,
    s2_range: [ 0xA1, 0x53, 0x78, 0xA4 ]
  }
}

type_overrides = {
  checksum: 'on_seq'
}

lengths = {
  color: 256,
  brightness: 0x64,
  kelvin: 0x64,
  saturation: 0x64,
  on_seq: 30,
  checksum: 0xFF,
  group: 40
}

columns = {
  on_seq: 6,
  group: 7,
  checksum: 8
}

extract_fns = {
  group: ->(b0) do
    get_group_sequence(type: 'on', col: 7, key: b0, group_range: (0..100))
  end
}

# PACKET_TYPES = %w(color brightness saturation kelvin on_seq checksum)
PACKET_TYPES = %w(color brightness kelvin saturation group)

(0x00..0xFF).each do |b0|
  k = xor_key(b0)
  results = {}
  
  PACKET_TYPES.each do |type|
    type_args = extra_args[type.to_sym] || {}
    file_type = type_overrides[type.to_sym] || type
    
    if extract_fn = extract_fns[type.to_sym]
      packet_str = extract_fn.call(b0)
    else
      packet_str = get_sequence(file_type, b0, columns[type.to_sym] || 5)
    end
    
    max_len = lengths[type.to_sym]*2
    if packet_str.length > max_len
      packet_str = packet_str[0...max_len]
    end
    
    args = {}.merge(extra_args[type.to_sym] || {})
    
    
    if args[:s1_range].is_a?(Proc)
      args[:s1_range] = args[:s1_range].call(b0)
    end
    
    if args[:s2_range].is_a?(Proc)
      args[:s2_range] = args[:s2_range].call(b0)
    end
    
    args = {
      seq: packet_str,
      xor_range: [k]
    }.merge(args)
    
    results[type] = search_sequence(args)
  end
  
  num_fns = results.values.map { |x| x[:scramble_fns].length }.max
  printf "0x%02X", b0
  
  (0...num_fns).each do |i|
    printf "\t\t"
    
    PACKET_TYPES.each do |type|
      result = results[type]
      
      fns = result[:scramble_fns]
      
      if i < fns.length
        fn = result[:scramble_fns][i]
        printf "0x%02X\t0x%02X\t0x%02X\t%d\t", fn[:a], fn[:x], fn[:b], result[:num_misses]
      else
        printf "\t\t\t\t"
      end
    end
      
    printf "\n"
  end
end

# Dir.glob("#{CAPTURES_DIR}/**.txt").each do |filename|
#   puts File.basename(filename)
#   capture = parse_capture(filename)
#   
#   pp first_byte_determines_next(capture, 3)
#   
#   # a = capture
#   #   .group_by { |x| x[0] }
#   #   .sort
#   #   .map { |_, packets| [packets.first[0], packets.first[1]] }
#   # mappings = Hash[a]
#   # perms = []
#   # current_perm = []
#   # elem = a.first
#   # current_perm = [elem[0]]
#   # visited = Set.new([elem[0]])
#   # perm_visited = Set.new([elem[0]])
#   # 
#   # while visited.length < mappings.length
#   #   new_elem = nil
#   #   
#   #   puts visited.inspect
#   #   
#   #   if !mappings.include?(elem[1]) || perm_visited.include?(mappings[elem[1]])
#   #     new_elem = (Set.new(mappings.keys) - visited).first
#   #     perms << current_perm
#   #     current_perm = []
#   #     perm_visited = Set.new
#   #   else
#   #     new_elem = [elem[1], mappings[elem[1]]]
#   #   end
#   #   
#   #   visited << new_elem[0]
#   #   perm_visited << new_elem[0]
#   #   elem = new_elem
#   # end
#   # 
#   # perms << current_perm
#   # 
#   # pp perms
#   
#   a = capture
#    .group_by { |x| x[0] }
#    .sort
#    .map { |_, packets| [packets.first[0], packets.first[1]] }
#    .sort { |x,y| x[1] <=> y[1] }
#   a = a
#    .each_with_index.map do |x, i|
#      [x[1],x[0],if a[i-1][1] == x[1]-1
#        (x[0]-a[i-1][0])
#      else
#        nil
#      end]
#    end
#    .to_a
#   #  # .group_by { |x| x[1] }
#   #  # .sort
#   #  # .to_a
#   
#   pp a
#   
#   # a = capture
#   #   .group_by { |x| x[0] }
#   #   .map { |_, packets| packets.first }
#   #   .sort #{ |x, y| x[1] <=> y[1] }
#   #   .map { |x| x }
#   #   .to_a
#   #   
# end

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