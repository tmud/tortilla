-- spit
-- Плагин для Tortilla mud client

local spit = {}
function spit.name() 
  return 'Отправляет текстовый файл в мад'
end

function spit.description()
  local s  = {
  'Плагин читает файл и отправляет его построчно в мад.',
  '#spit <файл> [строка для подстановки] [номера строк]',
  'Плагин работает с файлами в кодировке win1251.',
  'Можно не указывать строку для подстановки и/или номера строк.',
  'Если в строке для подстановки используется %0, то строки из файла подставляютca вместо %0.',
  'Номера строк можно указывать списком или диапазонами, например: 1 | 1,2,5 | 1-6'
  }
  return table.concat(s, '\r\n')
end

function spit.version()
  return '1.0'
end

local function print(s)
  return _G.print('[spit] '..s)
end

local number, diapozon, templ
function spit.init()
  if not system or not system.loadTextFile then
    terminate('Для работы плагина нужен модуль system.loadTextFile')
  end
  addCommand("spit")
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
    out[1] = template..' '
    out[2] = ''
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
  print('Формат команды: '..props.cmdPrefix()..'spit <файл> [строка для подстановки] [номера строк]')
end

function spit.syscmd(t)
  if t[1] ~= 'spit' then return t end
  if #t == 1 then printhelp() return end
  local f = system.loadTextFile(t[2])
  if not f then
    print('Не удалось прочитать файл: '..t[2])
    return
  end
  if #t == 2 then
    send(f, '')
    return
  end
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

return spit
