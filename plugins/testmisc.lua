-- testmisc
-- Плагин для Tortilla mud client

local testmisc = {}
function testmisc.name()
  -- плагин отключен в клиенте
  -- return 'Тесты команд и скриптов'
end

function testmisc.version()
  return '-'
end

function testmisc.description()
  return "Плагин для тестирования различных команд и скриптов"
end

function testmisc.init()
  addCommand('test')
end

function testmisc.syscmd(t)
  local c = t[1]
  if c == 'test' then
    local v = tonumber(t[2])
    if isViewVisible(v) then
      hideView(v)
    else
      showView(v)
    end
    return nil
  end
  return t
end

return testmisc