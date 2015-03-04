-- historyfilter
-- Плагин Tortilla mud client
-- Список команд, которые нужно блокировать
local cmds_list_for_block = {'с','ю','з','в','вв','вн' }

historyfilter = {}
function historyfilter.name() 
    return 'Фильтр истории команд'
end

function historyfilter.description()
local cmds = ''
for i = 1,#cmds_list_for_block do
if i ~= 1 then cmds = cmds..', ' end
cmds = cmds..cmds_list_for_block[i]
end
return 'Плагин блокирует попадание отдельных команд в историю.\r\n\z
Настройки задаются прямо в файле плагина plugins/historyfilter.lua\r\n\z
Текущие настройки фильтра: '..cmds
end

function historyfilter.version()
    return '-'
end

function historyfilter.historycmd(cmd)
for i = 1,#cmds_list_for_block do
  if cmds_list_for_block[i] == cmd then return false end
end
return true
end
