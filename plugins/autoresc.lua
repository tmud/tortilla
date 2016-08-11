-- Плагин автореска для Былин

-- Параметры для редактирования
-- Кого спасать (имена), плагин будет работать только с этими именами
local chars = {
  'Маша',
  'Саша',
  'Мара',
}
-- Кого спасать (имена в винительном падеже, обязательно в том же порядке, что и chars!)
local charsv = {
  'Машу',
  'Сашу',
  'Мару',
}
-----------------------------------------------------------------------
-- Работа со списком триггеров, создание. выключение всех, включение всех
local triggers_list = {}
local function trigger(str, fun)
  local t = createTrigger(str, fun)
  if not t then
    log("Ошибка в триггере: '"..str.."'")
  else
    triggers_list[#triggers_list+1] = t
  end
end
local function disable()
  for _,t in ipairs(triggers_list) do t:disable() end
  print('АВТОРЕСК ОФФ')
end
local function enable()
  for _,t in ipairs(triggers_list) do t:enable() end
  print('АВТОРЕСК ВКЛ')
end
-----------------------------------------------------------------------
-- Списки ресков
local resc = {}
local fullresc = {}
local klon = {}
local fullklon = {}

-- Функция очищает все рески
local function clear_rescs()
  resc = {}
  fullresc = {}
  klon = {}
  fullklon = {}
  print('АВТОРЕСКИ ОЧИЩЕНЫ')
end

-- Отображение ресков
local function show_rescs()
  local t = {}
  local st = { resc, fullresc, klon, fullklon }
  for i,s in ipairs(st) do
    for n,_ in pairs(s) do
      local nt = t[n]
      if not nt then nt = {} t[n] = nt end
      if i==1 then nt[#nt+1] = 'реск' end
      if i==2 then nt[#nt+1] = 'фул реск' end
      if i==3 then nt[#nt+1] = 'реск клон' end
      if i==4 then nt[#nt+1] = 'фул реск клон' end
    end
  end
  for n,nt in pairs(t) do
    print('[реск] '..n..': '..table.concat(nt,', '))
  end
end

-- Функция получает из vd имя цели и проверяет по списку разрешенных имен,
-- переводит из винительного в именительный падеж
local function get_name(vd)
  vd:select(1)
  -- берем параметр %1
  local s=vd:getParameter(1)
  s = s:lfup()
  for i,n in ipairs(charsv) do
    if n == s then return chars[i] end
  end
  for _,n in ipairs(chars) do
    if n == s then return n end
  end
end

-- Функции, которые вызовутся при срабатывании триггера
local function full_auto_resc(vd)
  local name = get_name(vd)
  if not name then return end  -- имя не из списка, ничего не делаем
  if fullresc[name] then
    runCommand('спас '..name)
  end
  if fullklon[name] then
    runCommand('прик всем спас '..name)
  end
end

local function auto_resc(vd)
  local name = get_name(vd)
  if not name then return end  -- имя не из списка, ничего не делаем
  if resc[name] then
    runCommand('спасти '..name)
  end
  if klon[name] then
    runCommand('прик всем спас '..name)
  end
end

local function kick(vd)
  local name = get_name(vd)
  if not name then return end  -- имя не из списка, ничего не делаем
  if resc[name] then
    runCommand('спасти '..name)
    runCommand('пнуть '..name)
  end
end

local cmdrxp, decl
local autoresc = {}
function autoresc.init()

  if not extra or not extra.declension then
    terminate("Для работы плагина нужен модуль extra.declension")
  end
  decl =  extra.declension()
  -- нормализация имен в настройках, пишем в словарь для подбора по короткому имени
  for i,n in ipairs(chars) do chars[i] = n:lfup() decl:add(chars[i]) end
  for i,n in ipairs(charsv) do charsv[i] = n:lfup() end

  -- создаем регулярку для фильтрации команд автореска
  cmdrxp = createPcre('(.*)реск(.*)')

  -- создаем триггера, цель атаки всегда в переменной %1
  trigger('Одним ударом %% повалил%% %1 на землю.', auto_resc)
  trigger('%% своим оглушающим ударом сбил%% %1 с ног.', auto_resc)
  trigger('%% завалил%% %1 на землю мощным ударом.', auto_resc)
  trigger('%% ловко подсек%% %1, уронив е%% на землю.', auto_resc)
  trigger('%1 замер%% на месте!', kick)
  trigger('%1 пошатнул%% от богатырского удара %%.', auto_resc)
  trigger('%1 ослеплен%% дыханием %%.', full_auto_resc)
  trigger('%1 медленно покрывается льдом, после морозного дыхания %%.', full_auto_resc)
  trigger('%1 бьется в судорогах от кислотного дыхания %%.', full_auto_resc)
  trigger('%1 подгорел%% в нескольких местах, когда %% дыхнул%% на %% огнем.', full_auto_resc)
  trigger('%% напустил%% газ на %1.', full_auto_resc)
  trigger('%% попытал%% подсечь %1, но упал сам.', full_auto_resc)
  trigger('%1 избежал попытки %% завалить его.', full_auto_resc)
  trigger('%% попытался завалить %1, но не тут-то было.', full_auto_resc)
  trigger('Удар %% прошел мимо %1.', full_auto_resc)
  trigger('%1 сумел%% избежать удара %%.', full_auto_resc)
  trigger('Попытка %% %1 оказалась неудачной.', full_auto_resc)
  trigger('%% попытал%% %2 %1, но %%.', full_auto_resc)
  trigger('%% клюнул%% %1.', full_auto_resc)
  trigger('%% ударил%% %1.', full_auto_resc)
  trigger('%% ободрал%% %1.', full_auto_resc)
  trigger('%% хлестнул%% %1.', full_auto_resc)
  trigger('%% рубанул%% %1.', full_auto_resc)
  trigger('%% укусил%% %1.', full_auto_resc)
  trigger('%% огрел%% %1.', full_auto_resc)
  trigger('%% сокрушил%% %1.', full_auto_resc)
  trigger('%% резанул%% %1.', full_auto_resc)
  trigger('%% оцарапал%% %1.', full_auto_resc)
  trigger('%% подстрелил%% %1.', full_auto_resc)
  trigger('%% пырнул%% %1.', full_auto_resc)
  trigger('%% уколол%% %1.', full_auto_resc)
  trigger('%% ткнул%% %1.', full_auto_resc)
  trigger('%% лягнул%% %1.', full_auto_resc)
  trigger('%% боднул%% %1.', full_auto_resc)
  trigger('%1 увернул%% от %%.', full_auto_resc)
  trigger('%1 сумел%% уклониться от %%.', full_auto_resc)
  trigger('Доспехи %1 полностью поглотили удар %%.', full_auto_resc)
  trigger('Магический кокон вокруг %1 полностью поглотил удар %%.', full_auto_resc)

  -- выключаем триггеры изначально
  disable()
end

-- обрабатывем команды
local function run(p1, p2, link)
  local t = decl:find(link)
  if not t then return end
  if #t ~= 1 then print('[реск] Уточните имя.') return end
  local name = t[1]
  name = name:lfup()
  local t, m
  if p1 == '' then t = fullresc; m = 'фул реск'
  elseif p1 == 'мини' then t = resc; m = 'мини реск'
  elseif p1 == 'к' then t = fullklon; m = 'фул реск клонами'
  elseif p1 == 'миник' then t = klon; m = 'мини реск клонами'
  end
  if t then
    if p2 == '+' then
      t[name] = true
      m = m..' '..name..' вкл'
    else
      t[name] = nil
      m = m..' '..name..' выкл'
    end
    print('[реск] '..m)
  end
end

function autoresc.gamecmd(t)
  if cmdrxp:find(t[1]) then
    if t[1] == 'рескаю' then enable() return end
    if t[1] == 'нерескаю' then disable() return end
    if t[1] == 'рескиочистить' then clear_rescs() return end
    if t[1] == 'рески' then show_rescs() return end
    local p1 = cmdrxp:get(1)  -- часть команды до реск
    local p2 = cmdrxp:get(2)  -- часть команды после реск
    if p1 == '' or p1 == 'мини' or p1 == 'миник' or p1 == 'к' then
      if p2 ~= '+' and p2 ~= '-' then
        print('[реск] формат: [мини|миник|к]реск(+|-) цель, рескаю, нерескаю, рески, рескиочистить')
        return
      end
      local link = t[2]
      if link then link = link:trim() end
      if not link or link:len() == 0 then
        print('[реск] Укажите цель.')
        return
      end
      run(p1, p2, link)
      return
    end
  end
  return t
end

function autoresc.name()
  return 'Автореск для Былин'
end
function autoresc.description()
  return 'Автореск, в этом плагине нужно указать имена кого нужно спасать.'
end
function autoresc.version()
  return '1.0'
end

return autoresc
