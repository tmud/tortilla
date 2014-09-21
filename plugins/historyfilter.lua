-- Плагин historyfilter для tortilla mud client
-- Список команд, которые нужно блокировать
local cmds_list_for_block = "с,ю,з,в,вв,вн"

historyfilter = {}
function historyfilter.name() 
    return 'Фильтр истории команд'
end

function historyfilter.description()
return 'Плагин блокирует попадание отдельных команд в историю.\r\n\z
Настройки задаются прямо в файле плагина plugins/historyfilter.lua\r\n\z
Текущие настройки фильтра: '..cmds_list_for_block
end

function historyfilter.version()
    return '1.0'
end

function historyfilter.historycmd()
--todo
system.test()

end
