-- Плагин autowrap для Tortilla mud client
-- Максимальная допустимая длина строки
local autowrap_maxlen = 100

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

function autowrap.div(v)
  local len,index = 0, 0
  for i=1,v:blocks() do
    local s = v:getblocktext(i)
    local newlen = len + s:len()
    if newlen > autowrap_maxlen then index=i; break; end
    len = newlen
  end
  if index == 0 then return end
  v:createstring()
  local new_string = v:getindex() + 1
  for i=index,v:blocks() do
    v:copyblock(i, new_string, i-index+1)
  end
  local ds = v:getblocktext(index)
  local p1len = autowrap_maxlen-len
  v:setblocktext(index, ds:substr(1, p1len))
  v:select(new_string)
  v:setblocktext(1, ds:substr(p1len+1, ds:len()-p1len))
end

function autowrap.after(window, v)
if window ~= 0 then return end
local count = v:size()
  local i = 1
  while i <= count do
    v:select(i)
    if v:gettextlen() > autowrap_maxlen then
      autowrap.div(v)
      count = v:size()
    end
    i = i + 1
  end
end
