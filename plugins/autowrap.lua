-- autowrap
-- Плагин для Tortilla mud client

-- Максимальная допустимая длина строк для главного окна, 0 - выключено
local autowrap_maxlen_main = 70
-- Максимальная допустимая длина строк для дополнительных output-окон, 0 - выключено
local autowrap_maxlen_out = 100

autowrap = {}
function autowrap.name()
    return 'Автоперенос строк'
end

function autowrap.description()
return 'Плагин автопереноса длинных строк. Строки длиннее, чем заданное число символов, будут\r\n\z
разбиты на две строки или более. Для главного окна это '..autowrap_maxlen_main..' символов,\r\n\z
для дополнительных это '..autowrap_maxlen_out..' символов. Нулевое значение - автоперенос выключен.\r\n\z
Настройки максимальной длины задаются прямо в файле плагина plugins/autowrap.lua'
end

function autowrap.version()
    return '-'
end

function autowrap.div(v, maxlen)
  local len,index = 0, 0
  local s
  for i=1,v:blocks() do
    s = v:getBlockText(i)
    local newlen = len + s:len()
    if newlen > maxlen then index=i; break; end
    len = newlen
  end
  --log(index)
  local pos = s:strstr(" ")
  log(pos)
  
  --[[
  if index == 0 then return end
  v:createString(v:isSystem(), v:isGameCmd())
  local new_string = v:getIndex() + 1
  for i=index,v:blocks() do
    v:copyBlock(i, new_string, i-index+1)
  end
  local ds = v:getBlockText(index)
  local p1len = maxlen-len
  v:setBlockText(index, ds:substr(1, p1len))
  v:select(new_string)
  v:setBlockText(1, ds:substr(p1len+1, ds:len()-p1len))
  ]]
end

function autowrap.after(window, v)
local maxlen
if window == 0  then
    if not autowrap_maxlen_main or autowrap_maxlen_main == 0 then 
      return
    end
    maxlen = autowrap_maxlen_main
else
    if not autowrap_maxlen_out or autowrap_maxlen_out == 0 then 
      return
    end
    maxlen = autowrap_maxlen_out
end
local count = v:size()
  local i = 1
  while i <= count do
    v:select(i)
    if v:getTextLen() > maxlen then
      autowrap.div(v, maxlen)
      count = v:size()
    end
    i = i + 1
  end
end