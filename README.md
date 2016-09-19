# mruby-remocon   [![Build Status](https://travis-ci.org/yamori813/mruby-remocon.svg?branch=master)](https://travis-ci.org/yamori813/mruby-remocon)
Remocon class

Bit Trade One IR Remocon controller

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
# Sony Analog TV ch +
on = [0x33, 0x09, 0x00]
# Sony Analog TV ch -
off = [0x33, 0x89, 0x00]
t.send(on)
usleep 500000
t.send(on)
sleep 5
t.send(off)
usleep 500000
t.send(off)
```

## License
under the MIT License:
- see LICENSE file
