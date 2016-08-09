local ingrs_level_s = { 'наилучшую', 'отличную', 'хорошую', 'среднюю', 'посрдественную', 'худшую' }
local ingrs_level_m = { 'наилучшие', 'отличные', 'хорошие', 'средние', 'посрдественные', 'худшие' }

-- мн.число, ед.число(вин.пад), цвет
local ingrs = {
  { 'ветки', 'ветку', color='green' },
  { 'грибы', 'гриб', color='red' },
  { 'жидкости', 'жидкость', color='magenta' },
  { 'крупы', 'крупу', color='cyan' },
  { 'металлы', 'металл', color='green' },
  { 'минералы', 'минерал', color='red' },
  { 'овощи', 'овощ', color='magenta' },
  { 'посуда', 'посуду', color='cyan' },
  { 'твари', 'тварь', color='green' },
  { 'трава', 'траву', color='red' },
  { 'ягоды', 'ягоду', color='magenta' }
}

local function f(x) 
  return x and '1' or '0'
end
local function print(x)
  _G.print('[автосбор]: '..x)
end
local function printc(x, ...)
  _G.print(x, ...)
end

local sbor = false
local cmdrxp, cmdrxp2, cmdrxp3, decl
local autosbor = {}
function autosbor.init()
  --[[if not extra or not extra.declension then
    terminate("Для работы плагина нужен модуль extra.declension")
  end
  decl =  extra.declension()
  ]]
  cmdrxp = createPcre('сбор(.*)')
  cmdrxp2 = createPcre('^(_[^+-]+)?([+-])$')
  cmdrxp3 = createPcre('^_([^0-9_]+)([0-9]|_все)$')
  for _,t in ipairs(ingrs) do
    local v = {}
    for j=1,6 do v[j] = false end
    t.val = v
    t.state = false
    local len = 12-t[1]:len(); local n = {}; for j=1,len do n[j] = ' ' end; n[2] = t[1]
    t.name = table.concat(n)
  end
end

local function runcmd(p)
  if p == '' then
    -- состояние сбора
    printc('cyan', f(sbor), '| автосбор  |наилуч(1)|отличн(2)|хороше(3)|средне(4)|посред(5)|худшее(6)|')
    for _,t in ipairs(ingrs) do
      local flags = ''; for j=1,6 do flags=flags..'    '..f(t.val[j])..'    |' end
      printc(t.color, f(t.state), '|'..t.name..'|'..flags)
    end
    return true
  end
  if not cmdrxp2:find(p) then
    print("НЕПРАВИЛЬНАЯ КОМАНДА':"..p)
    return
  end
  local mode=cmdrxp2:get(2)=='+' and true or false
  print("mode: "..tostring(mode))
  if cmdrxp2:get(1) == '' then
    -- весь сбор вкл/выкл
    sbor = mode
    if mode then 
        print('БУДУ СОБИРАТЬ')
    else
        print('НЕ БУДУ СОБИРАТЬ')
    end
    return true
  end
  local processed = false
  print(cmdrxp2:get(1))
  if cmdrxp3:find(cmdrxp2:get(1)) then
    --if cmdrxp3:get_
  end
  if not processed then
    print('НЕПРАВИЛЬНАЯ КОМАНДА')
  end
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
  'Плагин автоматически собирает ингридиенты (траву, грибы, ветки, жидкости, крупы, металлы,',
  'минералы, овощи, посуду, отвары, ягоды) различного качества.',
  'Команда: сбор[_что][качество]+-',
  'Вывод текущего статуса автосбора: сбор',
  'Включить или выключить автосбор полностью: сбор+, сбор-',
  'Сбор отдельного игридиента: сбор_трава+, сбор_ветки-',
  'Сбор отдельного игридиента и качества: сбор_ягоды3+, сбор_грибы2-',
  'Сбор отдельного игридиента всех качеств: сбор_минералы_все+, сбор_овощи_все-',
  }
  return table.concat(s, '\r\n')
end
function autosbor.version()
  return '1.0'
end

return autosbor
