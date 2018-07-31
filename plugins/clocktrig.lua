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
  p..'clocktrig add time { commands }- добавить команду '
  --'Удобно использовать, если игровые команды используются в таймерах или триггерах.',
  --"Список команд находится в файле: "..getPath('config.lua') }
  return table.concat(s, '\r\n')
end

function clocktrig.version()
  return '1.0'
end

local function clocks_list()
  print("Таймеров РЛ нет")
end

function clocktrig.syscmd(t)
  if t[1] ~= 'clocktrig' then
      return t
  end
  if #t == 1 then
    clocks_list()
    return
  end
  if t[2] == 'add' then
  end
  
end

return clocktrig
