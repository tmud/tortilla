-- autowrap
-- Плагин для Tortilla mud client
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
    local s = v:getBlockText(i)
    local newlen = len + s:len()
    if newlen > autowrap_maxlen then index=i; break; end
    len = newlen
  end
  if index == 0 then return end
  v:createString()
  local new_string = v:getIndex() + 1
  for i=index,v:blocks() do
    v:copyBlock(i, new_string, i-index+1)
  end
  local ds = v:getBlockText(index)
  local p1len = autowrap_maxlen-len
  v:setBlockText(index, ds:substr(1, p1len))
  v:select(new_string)
  v:setBlockText(1, ds:substr(p1len+1, ds:len()-p1len))
end

function autowrap.after(window, v)
if window ~= 0 then return end
local count = v:size()
  local i = 1
  while i <= count do
    v:select(i)
    if v:getTextLen() > autowrap_maxlen then
      autowrap.div(v)
      count = v:size()
    end
    i = i + 1
  end
end
