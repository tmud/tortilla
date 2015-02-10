-- Плагин Status Bar для Tortilla mud client
-- Список команд, которые нужно блокировать

local msgbox = system.msgbox

statusbar = {}
function statusbar.name() 
    return 'Плагин гистограммы'
end

function statusbar.description()
return 'Плагин отображает информацию о здоровье, мане, энергии в\r\n\z
виде полосок на отдельной панели клиента.'
end

function statusbar.version()
    return '1.0'
end

function statusbar.init()
  local p = createPanel("bottom", 32)
  local r = p:render()
  r:setbackground(120, 140, 160)
end
