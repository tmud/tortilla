-- colorgamecmd
-- Плагин для Tortilla mud client
-- Цвет из палитры клиента (0-255)
local color = 3

colorgamecmd = {}
function colorgamecmd.name() 
    return 'Подсветка игровых и системных команд'
end

function colorgamecmd.description()
return 'Плагин изменяет цвет игровых и системных команд, которые вводит игрок.'
end

function colorgamecmd.version()
    return '-'
end

function colorgamecmd.after(window, v)
if window ~= 0 then return end
  for i=1,v:size() do
    v:select(i)
    if v:isGameCmd() then
      local last=v:blocks()
      v:set(last,"textcolor",color)
    end
  end
end
