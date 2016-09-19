# mruby-remocon   [![Build Status](https://travis-ci.org/yamori813/mruby-remocon.svg?branch=master)](https://travis-ci.org/yamori813/mruby-remocon)
Remocon class
## install by mrbgems
- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :github => 'yamori813/mruby-remocon'
end
```
## example
```ruby
t = Remocon.new()
on = [0x33, 0x09, 0x00]
off = [0x33, 0x89, 0x00]
t.send(on)
sleep 1
t.send(on)
sleep 5
t.send(off)
sleep 1
t.send(off)
```

## License
under the MIT License:
- see LICENSE file
