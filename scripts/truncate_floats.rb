#!/usr/bin/env nwn-dsl

# This truncates position floats to a sane width, thus avoiding
# miniscule floating point differences in version control diffs.

# This is a filter script 
# nwn-gff -i t.git -o t.git.yml -l gff -k yaml -r truncate_floats.rb

PRECISION = 4

count = 0

self.each_by_flat_path do |label, field|
  next unless field.is_a?(Gff::Field)
  next unless field.field_type == :float
  #puts "#{label}"

  field.field_value =
    ("%.#{PRECISION}f" % field.field_value).to_f
  

  if label.end_with? "/Bearing" 
    #if field.field_value.to_s == "-3.1416"
    if field.field_value == -3.1416
      # puts "got bearing = -3.1416"
      field.field_value = 3.1416
    end
  end
  
  count += 1
end

log "#{count} floats truncated."
