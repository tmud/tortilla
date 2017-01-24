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

local onoff = false
local counter = 0
local maxcounter = 5

function testtray.syscmd(t)
  if t[1] ~= 'testtray' then
      return t
  end
  if not onoff then
    onoff = true
    counter = 0
    print('Тест tray Включен')
  else
    onoff = false
    print('Тест tray Выключен')
  end
end

function testtray.tick()
  if onoff then
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
