local triggers = {}
function triggers.name()
  return 'Набор триггеров'
end
function triggers.description()
  return 'Сложные триггеры.'
end
function triggers.version()
  return '-'
end

local function in_trigger(vd)
  -- триггер сработал
  -- выбираем 1 строку
  vd:select(1)
  -- берем параметр %1
  local s=vd:getParameter(1)
  -- разделяем на части по пробелу
  local t=s:tokenize(" ")
  -- берем первую часть до первого пробела
  local p1 = t[1]
  -- Обрезаем последние 2 символа
  local name = p1:substr(1, p1:len()-2)
  -- Отображаем в плагин-канал что получилось
  log(name)
  -- Запускаем команду удара
  runCommand("убить "..name)
end

function triggers.init()
  -- создаем триггер
  createTrigger("^%1 сражается с %2", in_trigger)

  -- проверяем триггер тестовой строкой
  runCommand("#output Огненный демон сражается с Иваном.")
end

return triggers