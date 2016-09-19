MRuby::Gem::Specification.new('mruby-remocon') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Hiroki Mori'
  spec.linker.libraries << ['usb']
end
