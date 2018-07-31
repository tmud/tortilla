-- clocktrig
-- Плагин Tortilla mud client

local clocktrig = {}
function clocktrig.name() 
  return 'Команды но часам реального мира'
end

function clocktrig.description()
  local p = props.cmdPrefix()
  local s = {
  'Плагин выполнения команд по часам реального мира.',
  p..'clocktrig - посмотреть список триггеров',
  p..'clocktrig help- справка по командам',
  p..'clocktrig add time { commands }- добавить триггер',
  p..'clocktrig del time { commands }- удалить триггер'
  }
  return table.concat(s, '\r\n')
end

function clocktrig.version()
  return '1.0'
end

local time_regexp
function clocktrig.init()
  addCommand("clocktrig")
  time_regexp=createPcre("^([0-9]+):([0-9]+)$")
end

local triggers={}
local function tr_list()
  local p = props.cmdPrefix()
  print ('Список триггеров: ('..p..'clocktrig help)')
  if #triggers == 0 then
    print("Триггеров РЛ нет")
  else
    for i,v in ipairs(triggers) do
      print(i)
    end
  end
end

local function tr_add(time, cmds)
  local h = triggers[time.h]
  if not h then 
    h = {}
    triggers[time.h] = h
  end
  h[time.m] = cmds
  return true
end

local function tr_del(time)
  local h = triggers[time.h]
  if not h then return false end
  if h[time.m] then
    h[time.m] = nil
    return true
  end
  return false
end

local function tr_help()
  local p = props.cmdPrefix()
  print(p.."clocktrig add time {commands} - добавить триггер")
  print(p.."clocktrig del time - удалить триггер")
  print("Формат времени time: 22:20, 9:40")
end

local function translate_time(s)
  if type(s) ~= 'string' then return end
  if not time_regexp:find(s) then return end
  local hours = time_regexp:get(1)
  local minutes = time_regexp:get(2)
  local h = tonumber(hours)
  local m = tonumber(minutes)
  if not h or not m then return end
  print(m)
  print(h)
  return { h=h, m=m }
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
    if #t ~= 4 then print("[clocktrig] add: Неверный набор команд.") return end
    local time = translate_time(t[3])
    if not time then print("[clocktrig] add: Неверный формат времени.") return end
    tr_add(time, t[4])
    return
  end
  if t[2] == 'del' then
    if #t ~= 3 then print("[clocktrig] del: Неверный набор команд.") return end
    local time = translate_time(t[3])
    if not time then print("[clocktrig] del: Неверный формат времени.") return end
    tr_del(time)
    return
  end
  print ("[clocktrig] Неверная команда.")
  tr_help()
end

return clocktrig
