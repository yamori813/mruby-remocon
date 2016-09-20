# mruby-remocon   [![Build Status](https://travis-ci.org/yamori813/mruby-remocon.svg?branch=master)](https://travis-ci.org/yamori813/mruby-remocon)
Remocon class

Bit Trade One IR Remocon controller

http://bit-trade-one.co.jp/BTOpicture/Products/005-RS/

This module make at FreeBSD and Mac OS X. May be work Linux.

This module use as follow repositorie source code.

https://github.com/kjmkznr/bto_ir_cmd

Mac OS X support is as follow site code.

http://dsas.blog.klab.org/archives/52097996.html

send method Array parameter

|offset|note|
|:--|:--|
|1 Byte hi 4 bit | length per 4 bit |
|1 Byte lo 4 bit | signal type. (1 AEHA, 2 NEC, 3 SONY, 4 MITSUBISHI)|
|2- Byte | data |

Sample data Sony Alalog TV Channel +

[0x33, 0x09, 0x00]

length is 3 * 4  = 12 bit : type is SONY : data is 0x090

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

# Sony Analog TV ch +
on = [0x33, 0x09, 0x00]
# Sony Analog TV ch -
off = [0x33, 0x89, 0x00]

t = Remocon.new()
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

## Todo

- error handling
- long data support
- receive support
