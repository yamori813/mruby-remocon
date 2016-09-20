MRuby::Gem::Specification.new('mruby-remocon') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Hiroki Mori'

  if (`uname`.chomp =~ /darwin/i) 
   spec.linker.flags << ['-framework IOKit', '-framework CoreFoundation']
  elsif (`uname`.chomp =~ /freebsd/i)
    spec.linker.libraries << ['usb']
  else
    spec.linker.libraries << ['usb-1.0']
  end
end
