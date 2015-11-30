-- bell
-- Плагин для Tortilla mud client

bell = {}
function bell.name() 
    return 'Озвучивание символа bell'
end

function bell.description()
return 'Плагин проигрывает звук, если сервер присылает символ ascii 0x7 (bell).'
end

function bell.version()
    return '-'
end

function bell.streamdata(s)
  local count = 1
  local sym = ':'
  local p = s:strstr(sym)
  while p do
    count = count + 1
	p = s:strstr(sym, p+1)
  end
  if count > 0 then
    system.beep(2000, 100)
  end
  return s
end
