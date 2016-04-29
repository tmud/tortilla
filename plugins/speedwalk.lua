-- speedwalk
-- Плагин для Tortilla mud client
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
  p..'swalk show name - показать маршрут на экране.'
 }
  return table.concat(s, '\r\n')
end
function speedwalk.version()
  return '1.0'
end

local recorddb = {}
local record = ""
local recording = false

local function print(s)
  _G.print("[swalk]: "..s)
end
local function output(s)
  _G.print(s)
end

function speedwalk.init()
  addCommand("swalk")
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
  print('Маршрут сохранен в базу. Запись остановлена.')
end

local function returnf(p)
  if not p then
    if not recording then
      print('Запись маршрута не осуществляется. Вернуться невозможно.')
    else
      local path = record
      stop_recording()
      play_path(path, true)
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
      print('Маршрут удален.')
    end
  end
end

local function show(p)
  if not p then
    print('Не указано имя маршрута.')
  else
    local path = recorddb[p]
    if not path then
      print('Такого маршрута нет.')
    else
      print(path)
    end
  end
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
  ['show'] = show 
}

function speedwalk.syscmd(t)
  if t[1] ~= 'swalk' then
   return t
  end
  local f
  local c = t[2]
  if c then f = cmds[c] end
  if not f then
    local p = props.cmdPrefix()
    print(p.."swalk start|stop|save|return|go|play|list|delete|show ("..p.."help speedwalk)")
    return {}
  end
  if #t > 3 then
    print('Слишком много параметров.')
    return {}
  end
  local p = t[3]
  f(p)
  return {}
end

function speedwalk.gamecmd(t)
  return t
end

return speedwalk
