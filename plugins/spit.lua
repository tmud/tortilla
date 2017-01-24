-- spit
-- Плагин для Tortilla mud client

local spit = {}
function spit.name() 
  return 'Отправляет текстовый файл в мад'
end

function spit.description()
  local p = props.cmdPrefix()
  local c = p..'spitdb '
  local s  = {
  'Плагин читает файл и отправляет его построчно в мад.',
  'Плагин работает с файлами в кодировке win1251.',
  p..'spit <файл> [строка для подстановки] [номера строк]',
  'Можно не указывать строку для подстановки и/или номера строк.',
  'Если в строке для подстановки используется %0, то строки из файла подставляютca вместо %0.',
  'Номера строк можно указывать списком или диапазонами, например: 1 | 1,2,5 | 1-6',
  c..'read <фaйл> <номер строки> <имя переменной> - читает строку в переменную,',
  'имя переменной нужно указывать без $. Значение используется через $var.',
  c..'write <фaйл> <номер строки> <текст> - записывает строку в нужную позицию',
  'в файле. Запись кэшируется. Сброс данных на диск происходит при закрытии плагина или',
  'клиента, а также при выполнении команды flush.',
  c..'flush <фaйл> - сбрасывает кэш(все что не записано) на диск.'
  }
  return table.concat(s, '\r\n')
end

function spit.version()
  return '1.02'
end

local function print(s)
  return _G.print('[spit] '..s)
end

local number, diapozon, templ
function spit.init()
  if not checkVersion or not checkVersion(1,0) then
    terminate('Для работы плагина требуется клиент версии 1.0+')
  end
  if not system then
    terminate('Для работы плагина нужен модуль system.')
  end
  addCommand("spit")
  addCommand('spitdb')
  number = createPcre('^[0-9]+$')
  diapozon = createPcre('^([0-9]+)-([0-9]+)$')
  templ = createPcre('%0')
end

local function getlines(p)
  if not p:only('0123456789-,') then return end
  local r = {}
  local t = p:tokenize(',')
  for _,s in ipairs(t) do
    if number:find(s) then
      r[#r+1] = tonumber(s)
    elseif diapozon:find(s) then
      local from = tonumber(diapozon:get(1))
      local to = tonumber(diapozon:get(2))
      if from > to then
        for i=from,to,-1 do r[#r+1] = i end
      else
        for i=from,to do r[#r+1] = i end
      end
    else
      return
    end
  end
  return r
end

local function sendline(out, line)
  local line = system.convertFromWin(line)
  local t = {}
  for j,s in ipairs(out) do
    if s == '' then 
      t[j] = line
    else
      t[j] = s
    end
  end
  runCommand(table.concat(t))
end

local function send(file, template, lines)
  local len = #file
  local out = {}
  if templ:findall(template) then
    local indexes = { 1 }
    for i = 1,templ:size()-1 do
      indexes[#indexes+1] = templ:first(i)
      indexes[#indexes+1] = templ:last(i)
    end
    indexes[#indexes+1] = template:len()+1
    local last = #indexes/2
    for i=1,last do
      local from = indexes[i*2-1]
      local from_len = indexes[i*2] - from
      if from_len > 0 then 
        out[#out+1] = template:substr(from, from_len)
      end
      if i ~= last then out[#out+1] = '' end
    end
  else
    if template == '' then
      out[1] = ''
    else
      out[1] = template..' '
      out[2] = ''
    end
  end
  if lines then
    for _,i in ipairs(lines) do
      if i>=1 and i<=len then
        sendline(out, file[i])
      end
    end
  else
    for _,line in ipairs(file) do
      sendline(out, line)
    end
  end
end

local function printhelp()
  local p=props.cmdPrefix()
  print('Формат команды: ')
  print(p..'spit <файл> [строка для подстановки] [номера строк]')
  print(p..'spitdb read|write|flush <файл> <номер строки> <перем|текст>')
end

local function writeFileTable(filename, t)
  return system.saveTextFile(filename, t)
end

local files = {}
local function writeFile(filename)
  local f = files[filename]
  if not f then return end
  writeFileTable(filename, f)
end

local function loadFile(filename, hideerr)
  local f = files[filename]
  if not f then
    f = system.loadTextFile(filename)
    if not f then
      if not hideerr then
        print('[textdb] Файл не найден: '..filename)
      end
      return
    end
    files[filename] = f
  end
  return f
end

function spit.release()
  for filename, t in pairs(files) do
    writeFileTable(filename, t)
  end
end

local function read(t)
  if #t ~= 5 then return false end
  local index = tonumber(t[4])
  if not index then return false end
  local f = loadFile(t[3])
  if not f then return true end
  if index >= 1 and index <= #f then
    vars:replace(t[5], f[index])
  else
    vars:replace(t[5], '')
  end
  return true
end

local function write(t)
  if #t ~= 5 then return false end
  local index = tonumber(t[4])
  if not index then return false end
  local f = loadFile(t[3], true)
  if not f then f = {} files[t[3]] = f end
  if index >= 1 then
    local from = #f+1
    for i=from,index-1 do f[i] = '' end
    f[index] = t[5]
    return true
  end
  return false
end

local function spitcmd(t)
  if #t == 1 then printhelp() return end
  local f = loadFile(t[2])
  if not f then return end
  if #t == 2 then send(f, '') return end
  if #t == 3 then
    local p = t[3]
    if p:find('%0') then
      send(f, t[3])
      return
    end
    local strings_list = getlines(t[3])
    if strings_list then
      send(f, '', strings_list)
      return
    end
    send(f, t[3])
    return
  end
  if #t == 4 then
    local strings_list = getlines(t[4])
    if strings_list then
      send(f, t[3], strings_list)
      return
    end
  end
  printhelp()
end

function spit.syscmd(t)
  if t[1] == 'spit' then
    spitcmd(t)
    return
  end
  if t[1] == 'spitdb' then
    if #t < 3 then printhelp() return end
    if t[2] == 'flush' then writeFile(t[3]) return end
    if t[2] == 'read' and read(t) then return end
    if t[2] == 'write' and write(t) then return end
    printhelp()
    return
  end
  return t
end

return spit
