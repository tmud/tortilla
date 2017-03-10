-- bellcmd
-- Плагин для Tortilla mud client

-- количество писков на одну команду
local count = 2

local bell2 = {}
function bell2.name()
  return 'Системная команда #bell'
end

function bell2.description()
  return 'Плагин проигрывает короткий звук по команде #bell.'
end

function bell2.version()
  return '1.02'
end

function bell2.init()
  addCommand('bell')
end

function bell2.syscmd(t)
  if t[1] ~= 'bell' then
   return t
  end
  if count > 0 then
    system.beep(10, 20)         -- для разгона звука, не убирать
  end
  for i=1,count do
    system.beep(2000, 150)
  end
  return nil
end

return bell2
