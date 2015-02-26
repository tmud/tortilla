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
  r:select(objs.pen1)
  r:select(objs.brush1)
  --r:rect{left = 10, right = 30, top = 10, bottom = 30}
  --r:rect{60, 10, 90, 27}
  r:solidrect{120, 0, 160, 30}
  r:select(objs.font1)
  r:print(10, 10, "Абырвалг")
end

function statusbar.init()
  local p = createPanel("bottom", 32)
  r = p:setrender(render)
  r:setbackground(140,190,130)  
  objs.pen1 = r:createpen{ style ="solid", width = 1, r = 0, g = 0, b = 120 }
  objs.brush1 = r:createbrush{ style ="solid", r = 200, g = 0, b = 200 }
  objs.font1 = r:createfont{ font="fixedsys", height = 11, bold = 0 }
end
