-- luatrigger
-- Плагин для Tortilla mud client

local luatrigger = {}
function luatrigger.name() 
    --return 'Lua Trigger'
end

function luatrigger.description()
return 'Плагин для тестирования функционала триггеров на Lua.'
end

function luatrigger.version()
    return '-'
end

local function event(v)
  v:select(1)
  print("cyan", "Сработал lua-триггер: "..v:getText())
  local count = v:parameters()
  print("cyan", "Параметры: "..count)
  if count > 0 then
    local p = ''
    for i=1,count do
	  if i~=1 then p = p..'|' end
	  p = p .. v:getParameter(i)
	end
	p = p .. "|"..tostring(v:isChanged())
	print("cyan", p)
  end
end

function luatrigger.init()
  createTrigger("%1) Windows(%2)", event)
  createTrigger("Using keytable", event)
  createTrigger({"Based On %1", "DikuMUD %1"}, event)  

  createTrigger({"", "Включена поддержка"}, event)
  createTrigger({"По вопросам (.*)", ""}, event)
end

return luatrigger