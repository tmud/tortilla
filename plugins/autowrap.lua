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
  local len,index = 0, 0
  for i=1,v:blocks() do
    local s = v:getblocktext(i)
    len = len + s:len()
    if len > autowrap_maxlen then index=i break end
  end
  if index == 1 then
    return
  end
  v:createstring()
  local new_string = v:getindex() + 1
  for i=index,v:blocks() do
    v:copyblock(i, new_string, i-index+1)
  end
  for i=v:blocks(),index,-1 do
    v:deleteblock(i)
  end
end

function autowrap.after(window, v)
if window ~= 0 then return end
local count = v:size()
  local i = 1
  while i <= count do
    v:select(i)
    i = i + 1
    if v:gettextlen() > autowrap_maxlen then
      div(v)
      count = v:size()
    end
  end
end
