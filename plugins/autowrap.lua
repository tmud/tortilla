-- Плагин autowrap для Tortilla mud client
-- Максимальная допустимая длина строки
local autowrap_maxlen = 80

autowrap = {}
function autowrap.name() 
    return 'Автоперенос строк'
end

function autowrap.description()
return 'Плагин автопереноса длинных строк для основного окна клиента.\r\n\z
Строки длиннее, чем '..autowrap_maxlen..' cимволов, будут разделены на две строки или более.\r\n\z
Настройка максимальной длины задается прямо в файле плагина plugins/autowrap.lua'
end

function autowrap.version()
    return '-'
end

local function div(v)
  log(v:gettext()..v:gettextlen())
end

function autowrap.after(window, v)
if window ~= 0 then return end
local count = v:size()
  for i=1,count do
    v:select(i)
    if v:gettextlen() > autowrap_maxlen then
      div(v)
      count = v:size()
    end
  end
end
