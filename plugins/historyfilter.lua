-- historyfilter
-- Плагин Tortilla mud client
local cmds_list_for_block = {}

local historyfilter = {}
function historyfilter.name() 
  return 'Фильтр истории команд'
end

function historyfilter.description()
  local desc = {
  'Плагин блокирует попадание отдельных команд в историю.',
  'Настройки задаются в файле: '..getPath('config.lua')
  }
  return table.concat(desc, '\r\n')
end

function historyfilter.version()
  return '1.0'
end

function historyfilter.init()
  local t = loadTable('config.lua')
  if not t then
    terminate('Нет файла настроек '..getPath('config.lua')..', со списком команд.')
  end
  if not t.cmdlist or type(t.cmdlist) ~= 'table' then
    terminate('Ошибка в настройках '..getPath('config.lua')..', нет списка команд cmdlist.')
  end
  cmds_list_for_block = {}
  for _,cmd in pairs(t.cmdlist) do
    cmds_list_for_block[cmd] = true
  end
end

function historyfilter.historycmd(cmd)
  if cmds_list_for_block[cmd] then return false end
  return true
end

return historyfilter