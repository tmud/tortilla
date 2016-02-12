-- autowrap
-- Плагин для Tortilla mud client
-- Минимальная длина 60 символов
-- Максимальная длина задается в параметрах ( 0 - автоперенос выключен, -1 - авторасчет максимальной длины по ширине окна ).
-- Авторасчет ширины окна приблизительный (различная ширина букв).

-- Максимально допустимая длина строк для главного окна
local autowrap_maxlen_main = -1
-- Максимально допустимая длина строк для output-окон
local autowrap_maxlen_out = -1

autowrap = {}
function autowrap.name()
  return 'Автоперенос строк'
end

function autowrap.version()
  return '1.03'
end

function autowrap.description()
local s = 'Плагин автопереноса длинных строк. Строки длиннее, чем заданное число символов, будут\r\nразбиты на две строки или более. Строки разбиваются по словам, если это возможно.\r\n'
  s = s..'Для главного окна сейчас '
if not autowrap_maxlen_main or autowrap_maxlen_main == 0 then
  s = s..'автоперенос выключен'
elseif autowrap_maxlen_main == -1 then
  s = s..'включен режим авторасчета максимальной длины строки'
else
  s = s..'задана максимальная длина '..autowrap_maxlen_main..' символов'
end
  s = s ..'.\r\nДля дополнительных окон сейчас '
if not autowrap_maxlen_out or autowrap_maxlen_out == 0 then
  s = s..'автоперенос выключен'
elseif autowrap_maxlen_out == -1 then
  s = s..'включен режим авторасчета максимальной длины строки'
else
  s = s..'задана максимальная длина '..autowrap_maxlen_out..' символов'
end
  s = s..'.\r\n'
  s = s..'Настройки максимальной длины задаются прямо в файле плагина plugins/autowrap.lua.\r\nПлагин позволяет изменять ширину текста в символах в окнах клиента командой:\r\n'
  s = s..props.cmdPrefix()..'linewidth окно ширина. Команда не включена в автоподстановку, так как не предназначена\r\nдля использования в триггерах.'
  return s
end

local function div(v, maxlen)
  local s= v:getText()
  local t = s:strall(" ")
  local minlen = maxlen - 15
  if minlen < 0 then minlen = 0 end
  local sym = maxlen
  for _,j in ipairs(t) do
    if j >= minlen and j <= maxlen then
      sym = j
    end
  end  
  local block,pos = v:getBlockPos(sym)
  v:createString(v:isSystem(), v:isGameCmd())
  local new_string = v:getIndex() + 1
  for i=block,v:blocks() do
    v:copyBlock(i, new_string, i-block+1)
  end
  for i=v:blocks(),block+1,-1 do
    v:deleteBlock(i)
  end
  local ds = v:getBlockText(block)
  if pos == 1 then
    v:deleteBlock(block)
  else
    v:setBlockText(block, ds:substr(1, pos))
  end
  v:setNext(true)
  v:select(new_string)
  if pos == ds:len() then
    v:deleteBlock(1)
  else
    v:setBlockText(1, ds:substr(pos+1, ds:len()-pos))  
  end  
  v:setPrev(true)
end

local function divall(v, maxlen)
  local i,size = 1,v:size()
  while i <= size do
    v:select(i)
    if not v:isDropped() and v:getTextLen() > maxlen then
      div(v, maxlen)
      size = v:size()
    end
    i = i + 1
  end
end

function autowrap.after(window, v)
  local maxlen
  if window == 0 then
    if not autowrap_maxlen_main or autowrap_maxlen_main == 0 then return; end
    if autowrap_maxlen_main == -1 then
        maxlen = getViewSize(window)
    else
        maxlen = autowrap_maxlen_main 
    end

  else
    if not autowrap_maxlen_out or autowrap_maxlen_out == 0 then return; end
    if autowrap_maxlen_out == -1 then
        maxlen = getViewSize(window)
    else
        maxlen = autowrap_maxlen_out
    end
  end
  if maxlen < 60 then maxlen = 60 end
  divall(v, maxlen)
end

local function update_view(v, newlen)
    -- собираем сначала все разбитые строки
    local i,size = 1,v:size()
    while i <= size do
      v:select(i)
      v:setPrev(false)
      if v:isNext() then
        v:setNext(false)
        local from,to = i,i
        for j=i+1,size do
          to=j
          v:select(j)
          if not v:isNext() then break; end
        end
        if from ~= to then
          v:select(from)
          local blocks = v:blocks()
          for j=from+1,to do
            v:select(j)
            for k=1,v:blocks() do
              blocks=blocks+1
              v:copyBlock(k, from, blocks)
            end
          end
          for j=to,from+1,-1 do
            v:select(j)
            v:deleteString()
          end
          size = v:size()
        end
      end
      i = i + 1
    end
    -- разбиваем по новой длине
    divall(v, newlen)
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
    local newlen = tonumber(t[3])
    if window < 0 or window > 6 then
        log("Указан неверный номер окна: "..window..", "..cmdstr(t))
        return false
    end
    if newlen < 60 then
        log("Указан неверный размер строки: "..newlen..", "..cmdstr(t))
        return false
    end
    updateView(window, function(v) update_view(v,newlen) end)
    return nil
end