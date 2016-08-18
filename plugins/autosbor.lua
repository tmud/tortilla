-- Символы вкл/выкл, отображаемые в общей таблице сбора
local sym_yes = '+'
local sym_no = ' '

-- цвет заголовка окна общего состояния сбора
local title_color = 'rgb240,120,80 b rgb90,40,20'

-- цвет текста для тега с качеством ресурса
local teg_color = 'light red b blue'

-- триггеры на ресурсы
------------------------------------------------------------------------------------
-- q6 худшие, q5 посредственные, q4 средние, q3 хорошие, q2 отличные, q1 наилучшие
local vetki = {
  ingr = 'ветки',
  trigger = 'Отломанная ветка %1 сохнет здесь.',
  q6 = { 'ольхи' },
  q5 = { 'ели', 'клена' },
  q4 = { 'липы', 'пихты', 'черемухи' },
  q3 = { 'бука', 'вяза', 'груши', 'яблони', 'ясеня' },
  q2 = { 'дуба', 'карагача', 'кедра', 'кипариса', 'тиса' },
  q1 = { 'ильма', 'каштана', 'платана', 'самшита' }
}
local griby = {
  ingr = 'грибы',
  trigger = 'Гриб (%1) растет здесь.',
  q6 = { 'волнушка', 'говорушка' },
  q5 = { 'масленок', 'подберезовик', 'рыжик', 'тонкая свинушка' },
  q4 = { 'дождевик', 'едкая сыроежка', 'печерица' },
  q3 = { 'белый груздь', 'красный мухомор', 'красный опенок', 'подосиновик', 'черный груздь' },
  q2 = { 'бледная поганка', 'боровик', 'жгучая сыроежка', 'подвишенник' },
  q1 = { 'белый мухомор', 'желтый ежевик', 'навозник', 'пятнистый мухомор', 'скрипица'}
}
local griby2 = {
  ingr = 'грибы',
  trigger = '%1 растут на пеньке.',
  q6 = { 'опята' }
}
local zidkosti = {
  ingr = 'жидкости',
  trigger = 'Лужица %1 разлита у ваших ног.',
  q6 = { 'дождевой воды', 'прокисшего пива', 'ржаного кваса' },
  q5 = { 'вина', 'ключевой воды', 'сметаны' },
  q4 = { 'медовухи', 'простокваши', 'самогона' },
  q3 = { 'кислого молока', 'клюквеного киселя', 'сбитня' },
  q2 = { 'масла', 'меда' },
  q1 = { 'дегтя', 'конопляного масла', 'крови', 'ртути' }
}
local krupy = {
  ingr = 'крупы',
  trigger = 'Немного %1 просыпано здесь.',
  q6 = { 'манки', 'отрубей' },
  q5 = {},
  q4 = { 'овса', 'ячменя' },
  q3 = { 'гороховой крупы', 'гречневой ядрицы' },
  q2 = { 'проса', 'толокна' },
  q1 = { 'гречки', 'ячневой крупы' }
}
local metall = {
  ingr = 'металлы',
  trigger = 'Маленький кусочек %1 валяется в пыли.',
  q6 = { 'олова' },
  q5 = {},
  q4 = {},
  q3 = { 'латуни', 'свинца' },
  q2 = { 'мышьяка', 'серебра', 'чугуна' },
  q1 = { 'платины' }
}
local metall2 = {
  ingr = 'металлы',
  trigger = '%1 еле заметен в грязи.',
  q1 = { 'золотой самородок' }
}
local mineraly = {
  ingr = 'минералы',
  trigger = 'Осколок %1 наполовину утоплен в почву.',
  q6 = { 'кремня' },
  q5 = {},
  q4 = { 'базальта' },
  q3 = { 'мрамора', 'халцедона', 'хризопраза' },
  q2 = { 'опала', 'турмалина' },
  q1 = { 'александрита', 'алмаза', 'граната', 'изумруда', 'рубина' }
}
local ovoshi = {
  ingr = 'овощи',
  trigger = 'Здесь лежит %1.',
  q6 = { 'морковь', 'редис' },
  q5 = { 'редька' },
  q4 = { 'горох', 'кабачок', 'репа', 'фасоль' },
  q3 = { 'петрушка', 'сладкий перец', 'укроп' },
  q2 = { 'баклажан', 'красный перец', 'цветная капуста' },
  q1 = { 'лук-порей', 'ревень', 'тыква' }
}
local posuda = {
  ingr = 'посуда',
  trigger = 'У ваших ног лежит %1.',
  q6 = { 'глиняная плошка', 'оловянная чашка' },
  q5 = {},
  q4 = { 'большой горшок', 'железная ступка' },
  q3 = { 'железный котел', 'маленький котелок' },
  q2 = { 'бронзовый котел', 'железный чан', 'медный котел', 'медный чан', 'оловянный котел' },
  q1 = { 'огромный медный котел', 'огромный серебряный котел', 'серебряный тигель' }
}
local tvari = {
  ingr = 'твари',
  trigger = 'Вы заметили %1.',
  q6 = { 'птенца воробушка' },
  q5 = { 'клопа-вонючку', 'крошечную змейку', 'ручейника' },
  q4 = { 'клопа-гладыша', 'муравьишку', 'светлячка' },
  q3 = { 'желтого жучка', 'жужелицу', 'лягушонка', 'уховертку' },
  q2 = { 'дохлую водомерку', 'медведку', 'синего жучка', 'слепня' },
  q1 = { 'землеройку', 'красного жучка', 'мокрицу', 'сороконожку' }
}
local tvari2 = {
  ingr = 'твари',
  trigger = '%1 извивается здесь.',
  q3 = { 'длинный червяк' }
}
local tvari3 = {
  ingr = 'твари',
  trigger = '%1 катит свой шар мимо.',
  q1 = { 'навозный жук' }
}
local tvari4 = {
  ingr = 'твари',
  trigger = '%1 шевелит длинными усами.',
  q2 = { 'рыжий таракан' }
}
local travy = {
  ingr = 'трава',
  trigger = 'Среди разнотравья вы заметили %1.',
  q6 = { 'авран', 'ромашку' },
  q5 = { 'дрок', 'льнянку', 'мяту', 'чагу' },
  q4 = { 'барвинок', 'девясил', 'купальницу', 'солодку', 'шалфей' },
  q3 = { 'арнику', 'борец', 'валериану', 'крапиву', 'прострел', 'толокнянку', 'тысячелистник', 'чистец' },
  q2 = { 'белену', 'василистник', 'душицу', 'зверобой', 'крушину', 'молочай', 'обвойник', 'плаун', 'чистотел' },
  q1 = { 'аир', 'бессмертник', 'вех ядовитый', 'горец перечный', 'спорынью', 'чабрец', 'ясенец' }
}
local yagody = {
  ingr = 'ягоды',
  trigger = 'Приглядевшись, вы видите ягоду %1.',
  q6 = { 'бирючины', 'брусники', 'бычьего языка', 'дикого винограда', 'шиповника' },
  q5 = { 'вишни', 'голубики', 'клещевины', 'клюквы', 'метельника', 'тиса' },
  q4 = { 'барбариса', 'бересклета', 'малины', 'остролиста', 'черной смородины' },
  q3 = { 'белладонны', 'боярышника', 'ежевики', 'жимолости', 'клубники', 'красной жимолости', 'можжевельника', 'паслена', 'черники' },
  q2 = { 'белой смородины', 'волчьего лыка', 'дрока', 'дурмана', 'земляники', 'ирги', 'красной смородины', 'ландыша', 'сизой ежевики' },
  q1 = { 'белой малины', 'брионии', 'золотой акации', 'кизила', 'костяники', 'омелы', 'падуба', 'черноплодной рябины' }
}

-- общий список всех триггеров
local triggers = { vetki, griby, griby2, zidkosti, krupy, metall, metall2, mineraly, ovoshi, posuda, tvari, tvari2, tvari3, tvari4, travy, yagody }
------------------------------------------------------------------------------------
-- качества в падежах
local level_s = { 'наилучшую', 'отличную', 'хорошую', 'среднюю', 'посрдественную', 'худшую' }
local level_m = { 'наилучшие', 'отличные', 'хорошие', 'средние', 'посрдественные', 'худшие' }
local male = { 'наилучший', 'отличный', 'хороший', 'средний', 'посредственный', 'худший' }
local female = { 'наилучшая', 'отличная', 'хорошая', 'средняя', 'посредственная', 'худшая' }
local neutral = { 'наилучшее', 'отличное', 'хорошее', 'среднее', 'посрдественное', 'худшее' }

-- мн.число, ед.число(вин.пад), ед.число(им.падеж), пол, цвет
local ingrs = {
  { 'ветки', 'ветку', 'ветка', gender=female, level=level_m, color='b blue' },
  { 'грибы', 'гриб', 'гриб', gender=male, level=level_m, color='' },
  { 'жидкости', 'жидкость', 'жидкость', gender=female, level=level_m, color='b blue' },
  { 'крупы', 'крупу', 'крупа', gender=female, level=level_m, color='' },
  { 'металлы', 'металл', 'металл', gender=male, level=level_m, color='b blue' },
  { 'минералы', 'минерал', 'минерал', gender=male, level=level_m, color='' },
  { 'овощи', 'овощ', 'овощ', gender=male, level=level_m, color='b blue' },
  { 'посуда', 'посуду', 'посуда', gender=female, level=level_s, color='' },
  { 'твари', 'тварь', 'тварь', gender=female, level=level_m, color='b blue' },
  { 'трава', 'траву', 'трава', gender=female, level=level_s, color='' },
  { 'ягоды', 'ягоду', 'ягода', gender=female, level=level_m, color='b blue' }
}

local function f(x) 
  return x and sym_yes or sym_no
end
local function print(x)
  _G.print('[автосбор]: '..x)
end
local function printc(x, ...)
  if not x or x =='' then _G.print(...) return end
  _G.print(x, ...)
end

local teg_text_color, teg_bkg_color
local delimeter
local function updateColors()
  delimeter = props.cmdSeparator()
  if type(teg_color) == 'string' then
    teg_text_color, teg_bkg_color = translateColors( teg_color, props.paletteColor(7), props.backgroundColor() )
  end
  if not teg_text_color then
    teg_text_color = props.paletteColor(7)
    teg_bkg_color = props.backgroundColor()
  end
end

-- добавление тега в строку с ресурсом
local function addteg(vd, ingr, quality, flag)
  local idx = vd:blocks()+1
  vd:setBlocksCount(idx)
  local t = vd:getBlockText(idx-1)
  vd:setBlockText(idx-1, t..' ')
  local prefix = flag and '' or '-'
  vd:setBlockText(idx, '['..prefix..ingr[3]..'/'..neutral[quality]..']')
  vd:set(idx, 'exttextcolor', teg_text_color)
  vd:set(idx, 'extbkgcolor', teg_bkg_color)
end

local sbor = false
local sumka

-- запуск команды на сбор ингредиента
local function collect(name, ingr, quality)
  local cmd = 'вз все.'..ingr[3]
  if sumka then
    cmd = cmd..delimeter..'пол все.'..ingr[3]..' '..sumka
  end
  runCommand(cmd)
end

-- обработка триггера на ингредиент
local function ingr_trigger(vd, ingr, qt)
  vd:select(1)
  local name = vd:getParameter(1)
  if not name then return end
  name = name:lower()
  -- определяем качество ресурса
  local quality
  for i=1,6 do
    local t = qt['q'..i]
    if t then
      for _,l in pairs(t) do
        if l == name then quality = i break end
      end
    end
  end
  if not quality then return end
  -- проверяем что мы вообще собираем этот ресурс и собираем нужного качества
  local sbor_flag = ingr.state and ingr.val[quality]
  -- добавляем тег на ресурс
  addteg(vd, ingr, quality, sbor_flag)
  -- проверяем что мы вообще собираем этот ресурс и собираем нужного качества
  if sbor_flag then collect(name, ingr, quality) end
end

local cmdrxp, cmdrxp2, cmdrxp3, decl
local autosbor = {}
function autosbor.init()
  if not extra or not extra.declension then
    terminate('Для работы плагина нужен модуль extra.declension')
  end
  decl = extra.declension()
  decl:add('_сумка')
  sbor = false
  for _,t in ipairs(ingrs) do
    decl:add(t[1])
    local v = {}
    for j=1,6 do v[j] = false end
    t.val = v
    t.state = false
    local len = 12-t[1]:len(); local n = {}; for j=1,len do n[j] = ' ' end; n[2] = t[1]
    t.tablename = table.concat(n)
  end
  -- загружаем настройки сбора
  local function read(t, p)
    return t[p] == true and true or false
  end 
  local vars = loadTable(getProfile()..".lua")
  if vars then
    local j=1
    while vars['i'..j] do
      local v = vars['i'..j]
      if v.name then
        local ingr
        for _,it in ipairs(ingrs) do
          if it[1] == v.name then ingr = it break end
        end
        if ingr then
          ingr.state = read(v, 'on')
          for i=1,6 do ingr.val[i] = read(v, 'q'..i) end
        end
      end
      j=j+1
    end
    sbor = read(vars, 'on')
    sumka = vars['sumka']
    if sumka == '' then sumka = nil end
  end
  cmdrxp = createPcre('сбор(.*)')
  cmdrxp2 = createPcre('^(_[^+-]+)?([+-])$')
  cmdrxp3 = createPcre('^_?([^0-9_]+)([0-9]|_все)?$')

  -- создает триггеры на ресурсы
  for _,t in pairs(triggers) do
    local ingr
    for _,i in ipairs(ingrs) do
      if t.ingr == i[1] then ingr = i break end
    end
    if not ingr then log('Не найден игредиент: '..tostring(t.ingr)) goto next end
    local tr = createTrigger(t.trigger, function(vd) if sbor then ingr_trigger(vd, ingr, t) end end)
    if not tr then log('Не создан триггер: '..tostring(t.trigger)) end
    ::next::
  end
  updateColors()
end

function autosbor.release()
  -- сохраняем настройки сбора
  local vars = {}
  vars.on = sbor
  vars.sumka = sumka and sumka or ''
  for idx,t in ipairs(ingrs) do
    local s = {}
    s.on = t.state
    s.name = t[1]
    for i=1,6 do s['q'..i] = t.val[i] end
    vars['i'..idx] = s
  end
  saveTable(vars, getProfile()..".lua")
end

-- обновление цвета после настроек
function autosbor.propsupdated()
  updateColors()
end

-- работа с падежами
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
    if not sumka then print('Собирается все в инвентарь.')
    else print('Собирается все в сумку: '..sumka) end
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
    local text = mode and '' or 'НЕ '
    print(text..'БУДУ СОБИРАТЬ РЕСУРСЫ')
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
          if mode and not ingr.state then
            text = text.." (общий сбор выключен!)"
          end
          print(text)
          for i=1,6 do ingr.val[i] = mode end
          return true
        end
        local quality = tonumber(count)
        if quality >=1 and quality <= 6 then
          local text = mode and '' or 'НЕ '
          text = text..'БУДУ СОБИРАТЬ '..l3(ingr,quality)
          if mode and not ingr.state then
            text = text.." (общий сбор выключен!)"
          end
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
    if tl > 2 then print('НЕПРАВИЛЬНАЯ КОМАНДА') return end
    local p
    if tl == 2 then
      p = t[2]:trim()
      if p == '' then p = nil end
    end
    local cmd = cmdrxp:get(1)
    cmd = cmd:trim()
    local t = decl:find(cmd)
    if t and t[1]=='_сумка' and cmd:len()>1 then
      if not p then
        print('ВСЕ БУДЕТ СОБИРАТЬСЯ В ИНВЕНТАРЬ.')
        sumka = nil
      else
        print('ВСЕ БУДЕТ СОБИРАТЬСЯ В СУМКУ: '..p)
        sumka = p
      end
      return
    end
    if p then print('НЕПРАВИЛЬНАЯ КОМАНДА') return end
    if runcmd(cmd) then return end
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
  'Задать, сбросить сумку для сбора: cбор_сумка мешок, сбор_сумка',
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
