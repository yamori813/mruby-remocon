##
## Remocon Test
##

assert("Remocon#hello") do
  t = Remocon.new "hello"
  assert_equal("hello", t.hello)
end

assert("Remocon#bye") do
  t = Remocon.new "hello"
  assert_equal("hello bye", t.bye)
end

assert("Remocon.hi") do
  assert_equal("hi!!", Remocon.hi)
end
