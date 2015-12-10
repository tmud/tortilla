-- voice
-- Плагин для Tortilla mud client

voice = {}
function voice.name() 
  return 'Синтезатор речи'
end

function voice.description()
  return 'Плагин синтезирует текст в речь.'
end

function voice.version()
    return '1.0'
end

function voice.init()
  addCommand('voice')
end

function voice.syscmd(t)
  local c = t[1]
  if c == 'voice' then
    local text = {}
    for i=2,#t do	  
	  text[i-1] = t[i]
	end
	lvoice.play(table.concat(text, ' '))
    return nil
  end
  return t
end
