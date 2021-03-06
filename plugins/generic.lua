﻿-- Tortilla mud client plugin
-- Общие триггеры у персонажей

local act, sub, gag, hl

local function triggers()
-- Тут! вписывать свои триггеры, сабы(цвет задавать не обязательно), фильтры и подсветки
-- act("триггер", "список команд")
-- sub("триггер", "замена" [,"цвет"])
-- gag("триггер")
-- hl("триггер", "цвет")
end

local generic = {}
function generic.init()
  if gentr then
    act=gentr.action
    sub=gentr.sub
    gag=gentr.gag
    hl=gentr.highlight
    triggers()
  else
    terminate("Не загружен модуль generic, который необходим для работы плагина.")
  end
end

function generic.name()
  return 'Плагин с общими триггерами'
end

function generic.description()
  local s = 
  {
    "Плагин с примером, как создавать общие триггеры для всех персонажей.",
    "Нужно сделать копию файла с другим именем и вписать свои триггеры по примеру.", 
    "Чтобы включить триггеры, нужно включить соответствующий плагин в профиле персонажа.",
    "Для отключения триггеров, нужно плагин выключить."
  }
  return table.concat(s, '\r\n')
end
function generic.version()
  return '-'
end

return generic
