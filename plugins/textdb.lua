-- textdb
-- Плагин для Tortilla mud client

local textdb = {}
function textdb.name() 
  return 'Текстовый файл как массив строк'
end

function textdb.description()
  local p = props.cmdPrefix()..'textdb '
  local s = {
  'Плагин позволяет работать с тектовым файлом как с массивом строк, он умеет читать и ',
  'записывать отдельные строки файла. Плагин работает с файлами в кодировке utf8.',
  p..'read <имя фaйла> <номер строки> <имя переменной> - читает строку в переменную,', 
  'имя переменной нужно указывать без $. Далее значение можно использовать через $var.',
  p..'write <имя фaйла> <номер строки> <текст> - записывает строку в нужную позицию', 
  'в файле. Запись кэшируется. Сброс данных на диск происходит при закрытии плагина или',
  'клиента, а также при выполнении команды flush.',
  p..'flush <имя фaйла> - сбрасывает кэш(все что не записано) на диск.',
  'Данные также будут сброшены при выключении плагина или закрытии клиента.'
  }
  return table.concat(s, '\r\n')
end

function textdb.version()
  return '1.0'
end

local function printhelp()
  local p = props.cmdPrefix()..'textdb '
  print('[textdb] Формат команды: '..p..'read|write|flush <имя файла> [строка] [переменная|текст]')
  print('[textdb] Cм. описание команда плагина в менеджере.')
end

function textdb.init()
  if not checkVersion(1,0) then
    terminate('Для работы плагина требуется клиент версии 1.0+')
  end
  if not system or not system.loadTextFile or not system.saveTextFile then
    terminate('Для работы плагина нужен модуль system.loadTextFile & saveTextFile')
  end
  addCommand('textdb')
end

local function writeFileTable(filename, t)
  return system.saveTextFile(filename, t)
end

local files = {}
local function writeFile(filename)
  local t = files[filename]
  if not f then return end
  writeFileTable(filename, t)
end

function textdb.release()
  for filename, t in pairs(files) do
    writeFileTable(filename, t)
  end
end

local function read(t)
  if #t ~= 5 then return false end
  local index = tonumber(t[4])
  if not index then return false end
  local filename = t[3]
  local f = files[filename]
  if not f then
    if not system.loadTextFile(filename) then
      f = {}
      if not writeFileTable(filename, f) then
        log('Ошибка при чтении файла: '..filename)
        return true
      end
    end
    files[filename] = f
  end
  if index >= 1 and index <= #f then
    vars.replace(t[5], f[index])
  else
    vars.replace(t[5], '')
  end
  return true
end

local function write(t)
  if #t ~= 5 then return false end
  return true
end

function textdb.syscmd(t)
  if t[1] ~= 'textdb' then return t end
  if #t < 3 then printhelp() return end
  if t[2] == 'flush' then writeFile(t[3]) return end
  if t[2] == 'read' and read(t) then return end
  if t[2] == 'write' and write(t) then return end
  printhelp()
end

return textdb
