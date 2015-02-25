-- Плагин Status Bar для Tortilla mud client
-- Список команд, которые нужно блокировать

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

local r = nil
local objs = {}

local function render()
  --r:clear(120, 140, 160)
  --local w = r:width()
  --local h = r:height()
  --r:rect(2,2,w-2,h-2,st.pen1)
end

function statusbar.init()
  local p = createPanel("bottom", 32)
  r = p:setrender(render)
  r:setbackground(140,190,130)  
  objs.pen1 = r:createpen{ style ="solid", width = 1, r = 100, g = 200, b = 100 }
end
