-- testapi
-- Плагин для Tortilla mud client

local testapi = {}
function testapi.name()
  return 'Различные тесты api клиента'
end
function testapi.version()
  return '-'
end
function testapi.description()
  return "Плагин для тестирования api клиента"
end

local function assert(s1, s2, m)
  if s1 ~= s2 then
    log("unit test ("..m..") FAILED: ["..tostring(s1).."],["..tostring(s2).."]")
  end
end

local function test1()
  local string1 = "Дом умельца [33067]"
  print(string1..',len='..string1:len())
  print ("1) "..string1:substr(1,string1:len()))
  print ("2) "..string1:substr(string1:len()-15,string1:len()))
  print ("3) "..string1:substr(string1:len()-12,string1:len()))
  print ("4) "..string1:substr(string1:len()-8,string1:len()))
  print ("5) "..string1:substr(string1:len()-8,3))
end

local function check(a, b, s)
  if a ~= b then
    log("unit test ("..s..") FAILED!")
  end
end

local function test_arrayxy()
  print("arrayxy tests")
  local a = array_class:new(10, 10)
  a:set(1,2,100)
  check(a:get(1,2), 100, 'a:get(1,2) = 100')
  a:set(12,5,200)
  check(a:get(12,5), nil, 'a:get(12,5) = nil')
  a:set(3,5,300)
  check(a:get(3,5), 300, 'a:get(3,5) = 300')
  local b = array_class:new(5, 5)
  a:set(2,1,400)
  b:set(2,1,300)
  check(a:get(2,1), 400, 'a:get(2,1) = 400')
  check(b:get(2,1), 300, 'b:get(2,1) = 300')
end

function testapi.init()
  test1()
  test_arrayxy()
  addCommand('testapi')
end

function testapi.syscmd(t)
  if t[1] ~= 'testapi' then return t end
  test1()
  test_arrayxy()
end


return testapi

