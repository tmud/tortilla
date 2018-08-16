-- clocktrig
-- Плагин Tortilla mud client

local clocktrig = {}
function clocktrig.name() 
  return 'Команды но часам реального мира'
end

function clocktrig.description()
  local p = props.cmdPrefix()..'clocktrig '
  local s = {
  'Плагин выполнения команд по часам реального мира.',
  p..'- посмотреть список триггеров',
  p..'help- справка по командам',
  p..'add time { commands } - добавить(изменить) триггер',
  p..'del time - удалить триггер',
  p..'on - включить триггеры',
  p..'off - выключить триггеры',
  "Формат времени time: 22:20, 9:40",
  'Триггеры работают только при наличия подключения к маду.',
  'Плагин сохраняет таймеры в файле настроек для персонажа,',
  'т.е у каждого персонажа свой список таймеров.'
  }
  return table.concat(s, '\r\n')
end

function clocktrig.version()
  return '1.0'
end

local function printh(s)
  print("[clocktrig] "..s)
end

local function format_time(t)
  local m = ""
  if t.m < 10 then m = "0" end
  local s = ""
  if t.s then
     if t.s < 10 then s = "0" end
     s = ":"..s..t.s
  end
  return ""..t.h..":"..m..t.m..s
end

local triggers={}
local triggers_mode=true

local time_regexp
function clocktrig.init()
  addCommand("clocktrig")
  time_regexp=createPcre("^([0-9]+):([0-9]+)$")
  local t = loadTable(getProfile()..".lua")
  if type(t) ~= 'table' then return end
  local mode = t.mode
  t.mode = nil
  triggers = t
  if mode == nil then mode = false end
  if type(mode) ~= 'boolean' then mode = true end
  triggers_mode = mode
end
function clocktrig.release()
  triggers.mode = triggers_mode
  saveTable( triggers, getProfile()..".lua")
  triggers.mode = nil
end

local function tr_list()
  local p = props.cmdPrefix()
  local sec, min, hr =  system.getTime()
  local mode = "включены"
  if not triggers_mode then mode = "выключены" end
  print ('Список триггеров: ('..mode..', текущее время: '..format_time{s=sec,m=min,h=hr}..')')
  local out = {}
  for h,t in pairs(triggers) do
    print(h)
    for m,cmds in pairs(t) do
      local s = "" 
      if h < 10 then s = " " end
      local k = s..format_time{ h = h, m = m }
      out[k] = cmds
    end
  end
  if next(out) == nil then
    print("Триггеров РЛ нет")
    return
  end
  for k,s in pairsByKeys(out) do
    print(k..": "..s)
  end
end

local function tr_add(time, cmds)
  local h = triggers[time.h]
  if not h then 
    h = {}
    triggers[time.h] = h
  end
  if not h[time.m] then
    printh("Триггер на "..format_time(time).." добавлен.")
  else
    printh("Триггер на "..format_time(time).." именен.")
  end
  h[time.m] = cmds
end

local function tr_del(time)
  local exist = true
  local h = triggers[time.h]
  if not h then 
    exist = false
  else
    if not h[time.m] then
      exist = false
    else
      h[time.m] = nil
    end
  end
  if exist then
    printh("Триггер на "..format_time(time).." удален.")
  else
    printh("Триггера на "..format_time(time).." нет.")
  end
end

local function tr_help()
  local p = props.cmdPrefix()..'clocktrig '
  print(p.."add time {commands} - добавить триггер")
  print(p.."del time - удалить триггер")
  print(p..'on - включить триггеры')
  print(p..'off - выключить триггеры')
  print("Формат(24h) времени time: 22:20, 9:40")
end

local function translate_time_string(s)
  if type(s) ~= 'string' then return end
  if not time_regexp:find(s) then return end
  local hours = time_regexp:get(1)
  local minutes = time_regexp:get(2)
  local h = tonumber(hours)
  local m = tonumber(minutes)
  if not h or not m then return end
  return { h=h, m=m }
end

local function translate_time(s, cmd)
  local t = translate_time_string(s)
  if not t then printh(cmd..": Неверный формат времени "..s) return end
  if t.h < 0 or t.h > 23 or t.m < 0 or t.m > 59 then printh(cmd..": Недопустимое значение для времени "..format_time(t)) return end
  return t
end

function clocktrig.syscmd(t)
  if t[1] ~= 'clocktrig' then
      return t
  end
  if #t == 1 then
    tr_list()
    return
  end
  if t[2] == 'help' then
    tr_help()
    return
  end
  if t[2] == 'add' then
    if #t ~= 4 then printh("add: Неверный набор параметров.") return end
    local time = translate_time(t[3], "add")
    if not time then return end
    tr_add(time, t[4])
    return
  end
  if t[2] == 'del' then
    if #t ~= 3 then printh("del: Неверный набор параметров.") return end
    local time = translate_time(t[3], "del")
    if not time then return end
    tr_del(time)
    return
  end
  if t[2] == 'on' then
    if #t ~= 2 then printh("on: Неверный набор параметров.") return end
    triggers_mode = true
    printh("Триггеры включены.")
    return
  end
  if t[2] == 'off' then
    if #t ~= 2 then printh("off: Неверный набор параметров.") return end
    triggers_mode = false
    printh("Триггеры выключены.")
    return
  end
  printh ("Неверная команда.")
  tr_help()
end

local last_triggered = {}
function clocktrig.tick()
  if not triggers_mode then return end
  local _, m, h =  system.getTime()
  if last_triggered.h == h and last_triggered.m == m then return end
  local t = triggers[h]
  if t then
    local cmds = t[m]
    if cmds then
      last_triggered.h = h
      last_triggered.m = m
      printh("Триггер "..format_time{h=h,m=m}..": "..cmds)
      runCommand(cmds)
    end
  end
end

return clocktrig
