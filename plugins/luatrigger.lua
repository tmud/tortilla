-- luatrigger
-- Плагин для Tortilla mud client

luatrigger = {}
function luatrigger.name() 
    return 'Lua Trigger'
end

function luatrigger.description()
return 'Плагин для тестирования функционала триггеров на Lua.'
end

function luatrigger.version()
    return '-'
end

local function event(v)
  v:select(1)
  log("Сработал lua-триггер: "..v:getText())
  --print("Сработал lua-триггер: ")
end

function luatrigger.init()
  createTrigger("Using keytable", event)
end
