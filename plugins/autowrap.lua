-- autowrap
-- Плагин для Tortilla mud client
-- Минимальная длина 60 символов
-- Максимальная длина задается в параметрах ( 0 - автоперенос выключен, -1 - авторасчет максимальной длины по ширине окна ).
-- Авторасчет ширины окна приблизительный (различная ширина букв).

-- Максимально допустимая длина строк для главного окна
local autowrap_maxlen_main = -1
-- Максимально допустимая длина строк для output-окон
local autowrap_maxlen_out = -1
-- Минимальная длина строки
local autowrap_minlen = 25
-- Склеивать параграфы (возможность тестируется)
local concat_paragraphs = true

local autowrap = {}
function autowrap.name()
  return 'Автоперенос строк'
end
function autowrap.version()
  return '1.06'
end
function autowrap.description()
  local p = ''
  if not autowrap_maxlen_main or autowrap_maxlen_main == 0 then p = 'автоперенос выключен'
  elseif autowrap_maxlen_main == -1 then p = 'включен режим авторасчета максимальной длины строки'
  else p = 'задана максимальная длина '..autowrap_maxlen_main..' символов'
  end
  local p2 = ''
  if not autowrap_maxlen_out or autowrap_maxlen_out == 0 then p2 = 'автоперенос выключен'
  elseif autowrap_maxlen_out == -1 then p2 = 'включен режим авторасчета максимальной длины строки'
  else p2 = 'задана максимальная длина '..autowrap_maxlen_out..' символов'
  end
  local s = { 'Плагин автопереноса длинных строк. Строки длиннее, чем заданное число символов, будут',
  'разбиты на две строки или более. Строки разбиваются по словам, если это возможно.',
  'Для главного окна сейчас: '..p..'.',
  'Для дополнительных окон сейчас: '..p2..'.',
  'Настройки максимальной длины задаются прямо в файле плагина plugins/autowrap.lua.',
  'Плагин позволяет изменять ширину текста в символах в окнах клиента командой:',
  props.cmdPrefix()..'linebreak окно ширина. Команда не включена в автоподстановку, так как не предназначена',
  'для использования в триггерах. Без параметров команда пересчитывает автоперенос',
  'по ширине окна.'
  }
  return table.concat(s, '\r\n')
end

local function print(s)
  _G.print('[автоперенос] '..s)
end
local function print0(s)
  _G.print(s)
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

local block_color = '7;0;;'
local function paragraph(v)
  local i,size = 1,v:size()
  while i < size do
    v:select(i)
    if v:blocks() == 1  and v:getBlockColor(1) == block_color then
      if v:isDropped() or v:isGameCmd() or v:isSystem() or v:isPrompt() then goto next end
      v:select(i+1)
      if v:blocks() == 1 and v:getBlockColor(1) == block_color then
        if v:isDropped() or v:isGameCmd() or v:isSystem() or v:isPrompt() then goto next end
        local t = v:getText()
        local first = t:substr(1, 1)
        if first:only('абвгдеёжзийклмнопрстуфкцчшщъыьэюя') then
          v:select(i)
          local t0 = v:getBlockText(1)
          local last = t0:substr(t0:len(), 1)
          if last ~= ' ' then 
            v:setBlockText(1, t0..' '..t)
          else
            v:setBlockText(1, t0..t)
          end
          v:select(i+1)
          v:deleteString()
          size = v:size()
          goto again
        end
      end
      ::next::
    end
    i = i + 1
    ::again::
  end
end

local function divall(v, maxlen)
  if concat_paragraphs then
    paragraph(v)
  end
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
  if maxlen < autowrap_minlen then
    maxlen = autowrap_minlen
  end
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

local function recalc_window(window, newlen)
  if not newlen then newlen = getViewSize(window) end
  if newlen < autowrap_minlen then
    newlen = autowrap_minlen
  end
  updateView(window, function(v) update_view(v,newlen) end)
end

function autowrap.syscmd(t)
    if t[1] ~= 'linebreak' then return t end
    local function isnumber(s)
        if type(s) == 'number' then return true end
        if type(s) ~= 'string' then return false end
        return tonumber(s) or true and false
    end
    local function cmdstr(t) 
        return "'"..props.cmdPrefix()..table.concat(t, " ").."'"
    end
    if #t == 1 then
      autowrap_maxlen_main = -1
      autowrap_maxlen_out = -1
      for i=0,6 do recalc_window(i) end
      return
    end
    local window = tonumber(t[2])
    if window < 0 or window > 6 then
      print("Указан неверный номер окна: "..window)
      return
    end
    if #t == 2 then
      if window == 0 then autowrap_maxlen_main = -1
      else autowrap_maxlen_out = -1 end
      recalc_window(window)
      return
    end
    if #t ~= 3 or not isnumber(t[2]) or not isnumber(t[3]) then
        print("Некорретный набор параметров: "..cmdstr(t))
        return
    end
    local newlen = tonumber(t[3])
    if newlen < autowrap_minlen then
      print("Указан неверный размер строки, минимально "..autowrap_minlen.." символов.")
      return
    end
    if window == 0 then autowrap_maxlen_main = newlen
    else autowrap_maxlen_out = newlen end
    recalc_window(window, newlen)
end

function autowrap.fontupdated()
  print('Изменился шрифт.')
  print('Если нужно пересчитать переносы, то')
  print0('выполните команду #linebreak. Эта операция может')
  print0('занять заметное время. Время зависит от количества')
  print0('текста в клиенте.')
end

return autowrap