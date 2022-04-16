-- testadd
-- Плагин для Tortilla mud client

local testadd = {}
function testadd.name()
  return 'Тесты добавления подсветок и прочего из луа'
end
function testadd.version()
  return '-'
end
function testadd.description()
  return "Плагин для тестирования добавления разных элементов"
end


function testadd.init()
  addCommand('testadd')
end

function testadd.syscmd(t)
  if t[1] ~= 'testadd' then
   return t
  end

  local x = highlights:add ("стоите", "rgb192,192,192 b rgb20,40,90") 
  log(x)

  return nil
end


return testadd
