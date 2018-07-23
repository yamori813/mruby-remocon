# mruby-remocon   [![Build Status](https://travis-ci.org/yamori813/mruby-remocon.svg?branch=master)](https://travis-ci.org/yamori813/mruby-remocon)
Remocon class

Bit Trade One IR Remocon controller

http://bit-trade-one.co.jp/BTOpicture/Products/005-RS/

This module make at FreeBSD and Mac OS X. May be work Linux.

This module use as follow repositorie source code.

https://github.com/kjmkznr/bto_ir_cmd

Mac OS X support is as follow library code.

https://github.com/signal11/hidapi

send method Array parameter

|offset|note|
|:--|:--|
|1 Byte hi 4 bit | length per 4 bit |
|1 Byte lo 4 bit | signal type. (1 AEHA, 2 NEC, 3 SONY, 4 MITSUBISHI)|
|2- Byte | data |

Sample data Sony CD Player Play

[0x33, 0x4d, 0x10]

length is 3 * 4  = 12 bit : type is SONY : data is 0x4d1

Data byte bit is revers from naitive data. Because of populer this format.

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
# use radi_SH firmware
# http://a-desk.jp/modules/forum_hobby/index.php?cat_id=8

# Sony CD Player Play
play = [0x33, 0x4d, 0x10]
# Sony CD Player Stop
stop = [0x33, 0x1d, 0x10]

t = Remocon.new()

if t.open then

  t.send(play)
  usleep 45*1000
  t.send(play)

  sleep 10

  t.send(stop)
  usleep 45*1000
  t.send(stop)
end
```

## License
under the MIT License:
- see LICENSE file

## Todo

- error handling
- long data support
- receive support
