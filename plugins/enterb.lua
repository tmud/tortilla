-- enterb
-- Плагин для Tortilla mud client

local enterb = {}
function enterb.name() 
  return 'Кнопка Enter'
end
function enterb.description()
   local s = { 'Плагин добавляет кнопку на панель, которая отправляет',
   'в мад содержимое командной строки клиента.',
   'Учитывает настройку - очищать или нет командную строку.' }
   return table.concat(s, "\r\n")
end
function enterb.version()
  return '-'
end

function enterb.init()
  if not checkVersion or not checkVersion(1,0) then
    terminate('Для работы плагина требуется клиент версии 1.0+')
  end
  addButton("plugins/enterb.bmp", 0, 1, "Отправить команду")
end

function enterb.menucmd()
  local command = getCommand()
  runCommand(command)
  if props.isClearCommandLine() then
    setCommand('')
  end
end

return enterb
