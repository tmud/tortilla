-- speedwalk
-- Плагин для Tortilla mud client

-- Используемые команды перемещений
local move_cmds = {
 n = { "с", "север", "n", "north" },
 s = { "ю", "юг", "s", "south" },
 w = { "з", "запад", "w", "west" },
 e = { "в", "восток", "e", "east" },
 u = { "вв", "вверх", "u", "up" },
 d = { "вн", "вниз", "d", "down" }
 }
-- Обратные направления
local rdir = {
  ['n'] = 's',
  ['s'] = 'n',
  ['w'] = 'e',
  ['e'] = 'w',
  ['u'] = 'd',
  ['d'] = 'u',
}

local speedwalk = {}
function speedwalk.name()
  return 'Маршруты speedwalks'
end
function speedwalk.description()
  local p = props.cmdPrefix()
  local s = { 'Плагин позволяет быстро перемещатся по миру по заданным маршрутам(speedwalk).',
  'Можно сохранять маршруты в базу и использовать позднее. Помогает находить обратную дорогу.',
  'Работает с мадами с 6 направлениями для передвижений - север,юг,запад,восток,вверх,вниз.',
  p..'swalk start - начать запись в память клиента перемещения от текущей комнаты.',
  p..'swalk stop - остановить запись, забыть маршрут.',
  p..'swalk save name - остановить запись, сохранить маршрут в базу под именем name.',
  p..'swalk return - вернуться по маршруту. Должна идти запись маршрута.',
  p..'swalk return name - не должно быть записи. Вернутся обратно по маршуту из базы с именем name.',
  p..'swalk go name - не должно быть записи. Идти по маршруту из базы по имени name.',
  p..'swalk play path - не должно быть записи. Идти по маршруту - одна буква - одно направление.',
  p..'swalk list - показать список маршрутов в базе.',
  p..'swalk delete name - удалить маршрут из базы.',
  p..'swalk show name - показать маршрут на экране.',
  p..'swalk add name path - добавить маршрут в базу.'
 }
  return table.concat(s, '\r\n')
end
function speedwalk.version()
  return '1.0'
end

local recorddb = {}
local record = ""
local recording = false
local cmap, blocked, move_queue

local function output(s)
  _G.print(s)
end
local function print(s)
  output("[swalk]: "..s)
end

function speedwalk.init()

  local x = { 1, 2, 3, 4, d={"aaaa", "bbb", c="ddd" }, "rrrr" }
  saveTable(x, 'test.lua')
  local x2 = loadTable('test.lua')
  local x3 = loadTable('test2.lua')
  saveTable(x3, 'test3.lua')

  local t = loadTable('config.lua')
  if not t or type(t.blocked) ~= 'table' then 
    terminate('Нет файла настроек.')
  end
  blocked = {}
  for _,b in pairs(t.blocked) do
    local pcre = createPcre(b)
    if not pcre then
      print('Регулярное выражение с ошибкой: '..b)
    else
      blocked[#blocked+1] = pcre
    end
  end
  addCommand("swalk")
  cmap = {}
  for k,cmds in pairs(move_cmds) do
    for _,cmd in pairs(cmds) do
      cmap[cmd] = k
    end
  end
  local routes = loadTable('routes.lua')
  if routes then
    recorddb = routes
  end
end

function speedwalk.release()
  saveTable(recorddb, 'routes.lua')
end

function speedwalk.disconnect()
  move_queue = nil
end

local function stop_recording()
  record = ""
  recording = false
end

local function start_recording()
  stop_recording()
  recording = true
end

local function play_path(path, backward_mode)
  local cmd = {}
  local count = ""
  for i=1,path:len() do
    local dir = path:substr(i, 1)
    if dir:only('0123456789') then
      count = count..dir
    elseif dir:only('nsweud') then
      if count == "" then count = "1" end
      if backward_mode then dir = rdir[dir] end
      for k=1,tonumber(count) do
        cmd[#cmd+1] = dir
      end
      count = ""
    else
      print('Некорректный путь.')
      return
    end
  end
  if backward_mode then
    local rcmd = {}
    local j = 1
    for i=#cmd,1,-1 do
      rcmd[j] = cmd[i]
      j = j + 1
    end
    cmd = rcmd
  end
  local runcmd = table.concat(cmd, props.cmdSeparator())
  --print(runcmd)
  runCommand(runcmd)
end

local function pop_last()
  local len = record:len()
  local last = record:substr(len,1)
  local i = len - 1
  if i == 0 then
    record = ""
    return { dir = last, count = 1 }
  end
  local p = record:substr(i,1)
  if not p:only('0123456789') then
    record = record:substr(len-1)
    return { dir = last, count = 1 }
  end
  repeat
    i = i - 1
    p = record:substr(i,1)
  until i == 0 or not p:only('0123456789')
  local count = record:substr(i+1, len-i-1)
  record = record:substr(len-count:len()-1)
  return { dir = last, count = tonumber(count) }
end

local function push_new(t)
  if t.count == 0 then return end
  if t.count == 1 then
    record = record..t.dir
  else
    record = record..t.count..t.dir
  end
end

local function record_dir(dir)
  local len = record:len()
  if len == 0 then
    record = record..dir
  else
    local last = record:substr(len,1)
    if last == rdir[dir] then
      local t = pop_last()
      t.count = t.count - 1
      push_new(t)
    elseif last == dir then
      local t = pop_last()
      t.count = t.count + 1
      push_new(t)
    else
      record = record..dir
    end
  end
  log('record: '..record)
end

local function collect_record_path()
  local path = ""
  local p = move_queue.first
  while p do
    path = path..p.dir
    p = p.next
  end
  return path
end

local function start(p)
  if recording then
    print('Начата запись нового маршрута.')
  else
    print('Начата запись маршрута.')
  end
  start_recording()
end

local function stop(p)
  if recording then
    stop_recording()
    print('Запись маршрута прекращена.')
  else
    print('Запись маршрута не производилась.')
  end
end

local function save(p)
  if not p then
    if recording then
      print('Укажите имя маршрута для записи.')
    else
      print('Запись маршрута не осуществляется. Не указано имя маршрута.')
    end
    return
  end
  if not recording then
    print('Запись маршрута не осуществляется. Маршрут не сохранен.')
    return
  end
  if record:len() == 0 then
    print('Маршрут не сохранен, так как он пустой.')
    return
  end
  recorddb[p] = record
  stop_recording()
  print('Маршрут '..p..' сохранен в базу. Запись остановлена.')
end

local function returnf(p)
  if not p then
    if not recording then
      print('Запись маршрута не осуществляется. Вернуться невозможно.')
    else
      local path = record
      stop_recording()
      play_path(path, true)
      start_recording()
    end
    return
  end
  if recording then
    print('Идет запись маршрута. Вернуться невозможно.')
  else
    local path = recorddb[p]
    if not path then
      print('Такого маршрута нет в базе. Вернуться невозможно.')
    else
      play_path(path, true)
    end
  end
end

local function go(p)
  if not p then
    if recording then
      print('Идет запись маршрута. Не указано имя маршрута.')
    else
      print('Не указано имя маршрута. Переместиться невозможно.')
    end
    return
  end
  if recording then
    print('Идет запись маршрута. Переместиться невозможно.')
  else
    local path = recorddb[p]
    if not path then
      print('Такого машрута нет в базе. Переместиться невозможно.')
    else
      play_path(path, false)
    end
  end
end

local function play(p)
  if not p then
    print('Не указан маршрут.')
    return
  else
    play_path(p, false)
  end
end

local function path(p)
  if not p then
    if recording then
      print('Идет запись маршрута. Переместиться невозможно.')
    else
      print('Не указан маршрут. Переместиться невозможно.')
    end
    return
  end
  if recording then
     print('Идет запись маршрута. Переместиться невозможно.')
  else
     play_path(path, false)
  end
end

local function list(p)
  print('Маршруты:')
  if not next(recorddb) then
    output('Маршрутов нет.')
    return
  end
  for k,_ in pairs(recorddb) do
    output(k)
  end
end

local function delete(p)
  if not p then
    print('Не указано имя маршрута.')
  else
    if not recorddb[p] then
      print('Такого маршрута нет.')
    else
      recorddb[p] = nil
      print('Маршрут '..p..' удален.')
    end
  end
end

local function show(p)
  if not p then
    if not recording then
      print('Запись маршрута не осуществляется.')
    else
      print(record)
    end
    return
  end
  local path = recorddb[p]
  if not path then
    print('Такого маршрута нет.')
  else
    print(path)
  end
end

local function add(p, path)
  recorddb[p] = path
  print('Маршрут '..p..' сохранен в базу.')
end

local cmds = {
  ['start'] = start,
  ['stop'] = stop,
  ['save'] = save,
  ['return'] = returnf,
  ['go'] = go,
  ['play'] = play,
  ['list'] = list,
  ['delete'] = delete,
  ['show'] = show,
  ['add'] = add
}

function speedwalk.syscmd(t)
  if t[1] ~= 'swalk' then return t end
  local f
  local c = t[2]
  if c then f = cmds[c] end
  if not f then
    local p = props.cmdPrefix()
    print(p.."swalk start|stop|save|return|go|play|list|delete|show|add ("..p.."help speedwalk)")
    return {}
  end
  if (#t > 3 and c ~= 'add') or #t > 4 then
    print('В команде слишком много параметров.')
  else
    f(t[3], t[4])
  end
  return {}
end

function speedwalk.gamecmd(t)
  if not recording then return t end
  if #t ~= 1 then return t end
  local dir = cmap[ t[1] ]
  if not dir then return t end
  local newmove = { dir = dir }
  if not move_queue then 
    move_queue = { first = newmove, last = newmove }
  else
    move_queue.last.next = newmove
    move_queue.last = newmove
  end
  --log("dir: "..collect_record_path())
  return t
end

local function popdir()
  local next = move_queue.first.next
  if not next then
    move_queue = nil
  else
    move_queue.first = next
  end
end

function speedwalk.before(v, vd)
  if not recording then return t end
  if v ~= 0 or not move_queue then return end
  for i=1,vd:size() do
    vd:select(i)
    if vd:isSystem() then goto next end
    if vd:isPrompt() then
      if move_queue then
        record_dir(move_queue.first.dir)
        popdir()
      end
    else
      for _,p in pairs(blocked) do
        if p:find(vd:getText()) then
          popdir()
        end
      end
    end
    ::next::
  end
end

return speedwalk
