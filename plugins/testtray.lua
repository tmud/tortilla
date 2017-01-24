-- testtray
-- Плагин Tortilla mud client

local testtray = {}
function testtray.name() 
  return 'Тестовый плагин для плагина tray'
end

function testtray.description()
  return 'Плагин используется для тестирования плагина tray.'
end

function testtray.version()
  return '-'
end

local counter
local maxcounter = 3

function testtray.syscmd(t)
  if t[1] ~= 'testtray' then
      return t
  end
  if t[2] then
    local v = tonumber(t[2])
    if v then maxcounter = v end
    counter = 0 
  else
    if counter then counter = nil
    else counter = 0
    end
  end
    
  if not counter then
    print('Тест tray Выключен')
  else
    print('Тест tray Включен, период='..maxcounter)
  end
end

function testtray.debugtick()
  if counter then
    if counter >= maxcounter then
      counter = 0
      local r = rnd.rand(0,9)
      local cmd = props.cmdPrefix()..'tray ['..r..']:'..rnd.string(50, 120)
      runCommand(cmd)
      return
    end
    counter = counter + 1
  end
end

return testtray
