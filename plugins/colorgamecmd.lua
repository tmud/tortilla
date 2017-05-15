-- colorgamecmd
-- Плагин для Tortilla mud client
-- Цвет из палитры клиента (0-255)
local color = 3

local colorgamecmd = {}
function colorgamecmd.name() 
    return 'Подсветка игровых и системных команд'
end

function colorgamecmd.description()
return 'Плагин подсвечивает цветом игровые и системные команды, которые вводит игрок.'
end

function colorgamecmd.version()
    return '1.02'
end

function colorgamecmd.after(window, v)
if window ~= 0 then return end
  for i=1,v:size() do
    v:select(i)
    if v:isGameCmd() then
      local last=v:blocks()
      if last > 0 then v:set(last,"textcolor",color) end
    end
  end
end

return colorgamecmd