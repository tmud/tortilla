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

function testapi.init()
  test1()
  addCommand('testapi')
end

function testapi.syscmd(t)
  if t[1] ~= 'testapi' then return t end
  test1()
end


return testapi

