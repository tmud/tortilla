﻿-- bell
-- Плагин для Tortilla mud client

bell = {}
function bell.name() 
    return 'Озвучивание символа bell'
end

function bell.description()
return 'Плагин проигрывает короткий звук, если сервер присылает символ ascii 0x7 (bell).'
end

function bell.version()
    return '1.0'
end

function bell.streamdata(s)
  local count = 0
  local sym = '/a' -- bell symbol
  local p = s:strstr(sym)
  while p do
    count = count + 1
    p = s:strstr(sym, p+1)
  end
  if count > 0 then
    system.beep(2000, 150)
  end
  return s
end