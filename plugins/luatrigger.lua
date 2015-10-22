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
  log("Сработал lua-триггер: "..v:getText())
end

function luatrigger.init()
  createTrigger("Please,", event)
end
