-- autowrap
-- Плагин для Tortilla mud client

-- Максимально допустимая длина строк для главного окна, 0 - выключено, минимум 60
local autowrap_maxlen_main = 70
-- Максимально допустимая длина строк для output-окон, 0 - выключено, минимум 60
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

function autowrap.syscmd(t)
    if t[1] ~= 'linewidth' then
        return t
    end
    local function isnumber(s)
        if type(s) == 'number' then return true end
        if type(s) ~= 'string' then return false end
        return tonumber(s) or true and false
    end
    local function cmdstr(t) 
        return "'"..props.cmdPrefix()..table.concat(t, " ").."'"
    end
    if #t ~= 3 or not isnumber(t[2]) or not isnumber(t[3]) then
        log("Некорретный набор параметров: "..cmdstr(t))
        return false
    end
    local window = tonumber(t[2])
    local maxlen = tonumber(t[3])
    if window < 0 or window > 6 then
        log("Указан неверный номер окна: "..window..", "..cmdstr(t))
        return false
    end
    if maxlen < 60 then
        log("Указан неверный размер строки: "..maxlen..", "..cmdstr(t))
        return false
    end
    
    
    return nil
end


local function div(v, maxlen)
  local s= v:getText()
  log(s)
  local t = s:strall(" ")
  local minlen = maxlen - 15
  if minlen < 0 then minlen = 0 end
  local sym = maxlen
  for k,v in ipairs(t) do
    if v >= minlen and v <= maxlen then
      sym = v
	end
  end
  local block,pos = v:getBlockPos(sym) 
  v:createString(v:isSystem(), v:isGameCmd())
  local new_string = v:getIndex() + 1
  for i=block,v:blocks() do
    v:copyBlock(i, new_string, i-block+1)
  end
  local ds = v:getBlockText(block)
  v:setBlockText(block, ds:substr(1, pos))
  v:select(new_string)
  v:setBlockText(1, ds:substr(pos+1, ds:len()-pos))
end

function autowrap.after(window, v)
local maxlen
if window == 0 then
  if not autowrap_maxlen_main or autowrap_maxlen_main == 0 then return; end
  maxlen = autowrap_maxlen_main  
else
  if not autowrap_maxlen_out or autowrap_maxlen_out == 0 then return; end
  maxlen = autowrap_maxlen_out
end
if maxlen < 60 then maxlen = 60 end
local count = v:size()
  local i = 1
  while i <= count do
    v:select(i)
    if v:getTextLen() > maxlen then
      div(v, maxlen)
      count = v:size()
    end
    i = i + 1
  end
end