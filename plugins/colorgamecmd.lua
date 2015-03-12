-- colorgamecmd
-- Плагин для Tortilla mud client

colorgamecmd = {}
function colorgamecmd.name() 
    return 'Подсветка игровых команд'
end

function colorgamecmd.description()
return 'Плагин красит игровые команды в требуемый цвет.'
end

function colorgamecmd.version()
    return '-'
end

function colorgamecmd.after(window, v)
if window ~= 0 then return end
local count = v:size()
  local i = 1
  while i <= count do
    v:select(i)
    if v:isGameCmd() then
	  local s = v:getBlockText(v:blocks)
	  local p=v:blocks()
	  v:set(p,"textcolor",6)
    end
    i = i + 1
  end
end
