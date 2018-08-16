-- Плагин с примерами триггеров

-- Работа со списком триггеров, создание. выключение всех, включение всех
-- Сделана только одна группа триггеров

local triggers_list = {}
local function trigger(str, fun)
  local t = createTrigger(str, fun)
  if not t then
    log("Ошибка в триггере: '"..str.."'")
  else
    triggers_list[#triggers_list+1] = t
  end
end
local function disable()
  for _,t in ipairs(triggers_list) do t:disable() end
  print('Триггеры ОФФ')
end
local function enable()
  for _,t in ipairs(triggers_list) do t:enable() end
  print('Триггеры ВКЛ')
end
-----------------------------------------------------------------------
-- Функции, которые вызовутся при срабатывании триггера
local function command(cmd)
  runCommand(cmd)
end

local example = {}
function example.init()
  trigger('%% своим оглушающим ударом сбил%% %1 с ног.', command('спасти'))
  -- выключаем триггеры изначально
  disable()
end

function example.gamecmd(t)
  if cmdrxp:find(t[1]) then
    if t[1] == 'example' then
      if t[2] == "1" then
        enable()
      end
      if t[2] == "0" then
        disable()
      end
    end
  end
  return t
end

function example.name()
  return 'Пример плагина с триггерами'
end
function example.description()
  return 'Описание что умент плагин'
end
function example.version()
  return '1.0'
end

return example
