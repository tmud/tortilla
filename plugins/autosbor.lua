-- Символы вкл/выкл, отображаемые в общей таблице сбора
local sym_yes = '+'
local sym_no = '-'

-- цвет заголовка окна общего состояния сбора
local title_color = 'rgb240,120,80'

local level_s = { 'наилучшую', 'отличную', 'хорошую', 'среднюю', 'посрдественную', 'худшую' }
local level_m = { 'наилучшие', 'отличные', 'хорошие', 'средние', 'посрдественные', 'худшие' }

-- мн.число, ед.число(вин.пад), цвет
local ingrs = {
  { 'ветки', 'ветку', level=level_m, color='light green' },
  { 'грибы', 'гриб', level=level_m, color='light red' },
  { 'жидкости', 'жидкость', level=level_m, color='light magenta' },
  { 'крупы', 'крупу', level=level_m, color='light cyan' },
  { 'металлы', 'металл', level=level_m, color='light green' },
  { 'минералы', 'минерал', level=level_m, color='light red' },
  { 'овощи', 'овощ', level=level_m, color='light magenta' },
  { 'посуда', 'посуду', level=level_s, color='light cyan' },
  { 'твари', 'тварь', level=level_m, color='light green' },
  { 'трава', 'траву', level=level_s, color='light red' },
  { 'ягоды', 'ягоду', level=level_m, color='light magenta' }
}

local function f(x) 
  return x and sym_yes or sym_no
end
local function print(x)
  _G.print('[автосбор]: '..x)
end
local function printc(x, ...)
  if not x then _G.print(...) return end
  _G.print(x, ...)
end

local sbor = false
local cmdrxp, cmdrxp2, cmdrxp3, decl
local autosbor = {}
function autosbor.init()
  if not extra or not extra.declension then
    terminate('Для работы плагина нужен модуль extra.declension')
  end
  decl =  extra.declension() 
  for _,t in ipairs(ingrs) do
    decl:add(t[1])
    local v = {}
    for j=1,6 do v[j] = false end
    t.val = v
    t.state = false
    local len = 12-t[1]:len(); local n = {}; for j=1,len do n[j] = ' ' end; n[2] = t[1]
    t.tablename = table.concat(n)
  end
  cmdrxp = createPcre('сбор(.*)')
  cmdrxp2 = createPcre('^(_[^+-]+)?([+-])$')
  cmdrxp3 = createPcre('^_?([^0-9_]+)([0-9]|_все)?$')
end

function autosbor.release()
  
end

local function l1(ingr, mode)
  if ingr.level == level_s then
    return mode and 'НЕКОТОРУЮ '..ingr[2] or ingr[2]
  end
  return mode and 'НЕКОТОРЫЕ '..ingr[1] or ingr[1]
end
local function l2(ingr, mode)
  if ingr.level == level_s then
    return mode and 'ВСЮ '..ingr[2] or 'ЛЮБУЮ '..ingr[2]
  end
  return mode and 'ВСЕ '..ingr[1] or 'ЛЮБЫЕ '..ingr[1]
end
local function l3(ingr, quality)
  local level = ingr.level
  if level == level_s then
    return level[quality]..' '..ingr[2]
  end
  return level[quality]..' '..ingr[1]
end

local function runcmd(p)
  if p == '' then
    -- общее состояние сбора
    printc(title_color, '| '..f(sbor), '| автосбор  |наилуч(1)|отличн(2)|хороше(3)|средне(4)|посред(5)|худшее(6)|')
    for _,t in ipairs(ingrs) do
      local flags = ''; for j=1,6 do flags=flags..'    '..f(t.val[j])..'    |' end
      printc(t.color, '| '..f(t.state), '|'..t.tablename..'|'..flags)
    end
    return true
  end
  if not cmdrxp2:find(p) then
    print('НЕПРАВИЛЬНАЯ КОМАНДА')
    return
  end
  local mode=cmdrxp2:get(2)=='+' and true or false
  if cmdrxp2:get(1) == '' then
    -- весь сбор вкл/выкл
    sbor = mode
    if mode then print('БУДУ СОБИРАТЬ')
    else print('НЕ БУДУ СОБИРАТЬ') end
    return true
  end
  if cmdrxp3:find(cmdrxp2:get(1)) then
    local ingr_name = cmdrxp3:get(1)
    local t = decl:find(ingr_name)
    if t then
      if #t ~= 1 then print('Уточните имя ресурса.') return true end
      ingr_name = t[1]
      local ingr
      for _,t in ipairs(ingrs) do
        if t[1] == ingr_name then ingr = t break end
      end
      if ingr then
        local count = cmdrxp3:get(2)
        if not count then 
          -- отдельный ресурс вкл/выкл
          local text = mode and '' or 'НЕ '
          text = text..'БУДУ СОБИРАТЬ '..l1(ingr,mode)
          print(text)
          local some = false; for i=1,6 do if ingr.val[i] then some = true break end end
          if mode and not some then
            print('НЕ ВЫБРАНО НИ ОДНОГО КАЧЕСТВА ДЛЯ ДАННОГО РЕСУРСА!')
          end
          ingr.state = mode
          return true
        end
        if count == '_все' then
          local text = mode and '' or 'НЕ '
          text = text..'БУДУ СОБИРАТЬ '..l2(ingr,mode)
          print(text)
          for i=1,6 do ingr.val[i] = mode end
          return true
        end
        local quality = tonumber(count)
        if quality >=1 and quality <= 6 then
          local text = mode and '' or 'НЕ '
          text = text..'БУДУ СОБИРАТЬ '..l3(ingr,quality)
          print(text)
          ingr.val[quality] = mode
          return true
        end
      end
    end
  end
  print('НЕПРАВИЛЬНАЯ КОМАНДА')
  return true
end

function autosbor.gamecmd(t)
  if cmdrxp:find(t[1]) then
    local tl = #t
    if tl == 1 or (tl == 2 and t[2]:trim() == '') then
      local cmd = cmdrxp:get(1)
      if runcmd(cmd:trim()) then return end
    end
  end
  return t
end

function autosbor.name()
  return 'Автосбор для Былин'
end
function autosbor.description()
  local s = {
  'Плагин автоматически собирает ресурсы (траву, грибы, ветки, жидкости, крупы, металлы,',
  'минералы, овощи, посуду, отвары, ягоды) различного качества.',
  'Команда: сбор[_что][качество]+-',
  'Вывод текущего статуса автосбора: сбор',
  'Включить или выключить автосбор полностью: сбор+, сбор-',
  'Сбор отдельного ресурса: сбор_трава+, сбор_ветки-',
  'Сбор отдельного ресурса и качества: сбор_ягоды3+, сбор_грибы2-',
  'Сбор отдельного ресурса всех качеств: сбор_минералы_все+, сбор_овощи_все-',
  }
  return table.concat(s, '\r\n')
end
function autosbor.version()
  return '1.0'
end

return autosbor
