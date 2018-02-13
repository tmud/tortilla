-- Плагин баф/дебаф для Палладина в Сфере Миров

-- Параметры для редактирования
-- check - команда для чека группы
-- timeout - частота проверки ( 0 - выключение проверки )
-- position - порядкоый номер буквы, отвечающей за позицию персонажей (рядом/не рядом),
-- Плагин сканирует строки из результата команды группы и считает буквы Н/Д, 
-- и ее порядкоый номер (считаются от 1) - это и есть нужное число. 
-- commands - массив команд для бафа/дебафа в порядке их запроса в команде check.
-- type - тип бафа ( true - на Д надо кастовать, false - на Н надо кастовать ).
-- Если рядом в середине массива, то нужно задать пустое значение '',true в commands,type
-- Учитывается, что вместо буквы Д может находится и таймер
-- recheck - если true, то команда чека для перепроверки бросается сразу после команды снятия, 
--   false - чек происходит только по таймеру

local checks = {

{ check = 'пг з м э холд молч слеп яд прок позиция рядом', timeout = 2, position = 6,
  commands = { 'очистить;встать;к своб.вол', 'очистить;встать;к сво.вол', 'очистить;встать;к слов.света', 'очистить;встать;к слов.жиз', 'очистить;встать;к обря_очищ' },
  type = { true, true, true, true, true },
  recheck = true
},
{ check = 'пг слабость слабоум рядом', timeout = 15, position = 3,
  commands = { 'очистить;встать;к сво.вол', 'очистить;встать;к сво.вол' },
  type = { true, true },
  recheck = false
},
{ check = 'пг благ рядом', timeout = 15, position = 2,
  commands = { 'очистить;встать;к свя.обр' },
  type = { false },
  recheck = false
},

}
-----------------------------------------------------------------------
local pt, ts0, ts, tt
local timers, current, torun

local function disable()
  timers = nil
  current = nil
  torun = nil
  print('Дебаф ОФФ')
end
local function enable()
  timers = {}
  for i,t in ipairs(checks) do
    if not t.check then
      log('В настройках у дебафа #'..i..' нет параметра check')
      goto next
    end
    if not t.timeout then
      log('В настройках у дебафа #'..i..' нет параметра timeout')
      goto next
    end
    if not t.position then
      log('В настройках у дебафа #'..i..' нет параметра position')
      goto next
    end
    if not t.commands then
      log('В настройках у дебафа #'..i..' нет параметра commands')
      goto next
    end
    if not t.type then
      log('В настройках у дебафа #'..i..' нет параметра type')
      goto next
    end
    if #t.commands ~= #t.commands then
      log('В настройках у дебафа #'..i..' нет соответствия commands и type')
      goto next
    end
    if t.timeout == 0 then goto next end
    timers[#timers+1] = { timer = t.timeout, cmd = t }
    ::next::
  end
  if #timers == 0 then
    timers = nil
    print('Дебаф ОФФ, нет ни одного правила, см. лог плагинов.')
  else
    print('Дебаф ВКЛ')
  end
end

local function filter(vs)
  if ts0:find(vs:getText()) then
    return true, false
  end
  return false,false
end
-----------------------------------------------------------------------
local pallad = {}
function pallad.init()
  -- создаем триггер на строку состояния в группе
  pt = prompt_trigger('^  Имя ', filter)
  ts0 = createPcre('^  .+(?: [НД] )')
  ts = createPcre(' [НД] |[0-9]{1,2}:[0-9]{2}')
  tt = createPcre('[0-9]+:[0-9]+')
  -- выключаем дебаф изначально
  disable()
end

function pallad.gamecmd(t)
  if #t == 1 then
    if t[1] == '+дебаф' then enable() return end
    if t[1] == '-дебаф' then disable() return end
    if t[1] == 'очистить' then
       current = nil
       return t
    end
  end
  return t
end

function pallad.before(v, vd)
  if pt and pt:check(vd) and current then
   local c = current
   local require = #c.commands + 1
   local affects = {}
   for _,s in ipairs(pt.strings) do
     if ts:findall(s:getText()) then
       local count = ts:size()-1
       if require ~= count then
         log('Количество Д|Н|таймеров ('..count..') != количеству команд ('..require..') для: '..current.check..'. Пропуск.')
         log('Ошибка в строке: '..s:getText())
         goto next
       else
         local v = {}
         affects[k] = v
         for i=1,count do
           if i == current.position and ts:get(i) == ' Д ' then
             v.near = true
           else
             if not v.cast then
               local type = c.type[i]
               if type and (ts:get(i) == ' Д ' or tt:find(ts:get(i)) ) then
                 v.cast = i
               end
               if type == false and ts:get(i) == ' Н ' then
                 v.cast = i
               end
             end
           end
         end
       end
     ::next::
     end
   end
   local tocast
   for k,v in ipairs(affects) do
     if v.near and v.cast then
       if not tocast or tocast > v.cast then tocast = v.cast end
     end
   end
   if not tocast then
     current = nil
     return
   end
   local commands = current.commands
   torun = { cmd = commands[tocast] }
   if current.recheck then
     torun.recheck = current
   end
   current = nil
  end
end

function pallad.tick()
  if torun then
    runCommand(torun.cmd)
    if torun.recheck then
      current = torun.recheck
      runCommand(current.check)
    end
    torun = nil
  end
  if current then return end
  if not timers then return end
  for i,t in ipairs(timers) do
    local s = t.timer - 1
    if s == 0 then
      current = t.cmd
      t.timer = current.timeout
      runCommand(current.check)
      return
    else
      t.timer = s
    end
  end
end

function pallad.release()
  disable()
end

function pallad.disconnect()
  disable()
  if pt then pt:disconnect() end
end

function pallad.name()
  return 'Автобаф палладина в Сфере Миров'
end
function pallad.description()
  local s = { 'Автоматический баф/дебаф для палладина в Сфере Миров.',
  'Работает в группе по таймеру проверяя состояние членов группы,',
  'и делает масс каст, если хотябы один согрупник рядом и требует каста.',
  '+дебаф - включение авторежима',
  '-дебаф - выключение авторежима,',
  'Настройка плагина в файле плагина, где есть подробное описание.'
  }
  return table.concat(s, '\r\n')
end
function pallad.version()
  return '0.7'
end

return pallad
