local testtr = {}
local view = 2

function testtr.name()
  return 'Плагин тестирования функции prompt_trigger'
end
function testtr.description()
  return 'Плагин для тестирования модуля prompt_trigger (trprompt.lua).'
end
function testtr.version()
  --off return '-'
end

local t
local function filter(vs)
  if vs:getTextLen() == 0 then return false end
  return true
end

function testtr.init()
  t = prompt_trigger('^Вы несете:', filter)
end

function testtr.before(v, vd)
  if v ~= 0 then return end
  if t and t:check(vd) then
    for _,s in ipairs(t.strings) do
      s:print(view)
    end
  end
end

function testtr.disconnect()
  if t then
    t:disconnect()
  end
end

return testtr