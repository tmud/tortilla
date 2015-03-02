-- Плагин Status Bar для Tortilla mud client

statusbar = {}
function statusbar.name() 
    return 'Гистограммы здоровья, маны и энергии'
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
local log = system.dbglog

function statusbar.render()
  r:select(objs.pen1)
  r:select(objs.brush1)
  --r:rect{left = 10, right = 30, top = 10, bottom = 30}
  --r:rect{60, 10, 90, 27}
  r:solidrect{10, 4, 160, 26}
  r:select(objs.font1)
  r:print(10, 10, "Абырвалг")
end

function statusbar.before(window, v)
if window ~= 0 then return end
local size = v:size()
for i=1,size do
  if (i==1 and v:isfirst()) then
    log("<CONT")
  end
  v:select(i)
  if (i==size) then
    log(">>>")
  end
  log(v:gettext())
  if (i==size) then    
    if (v:islast() or v:isprompt()) then
	   log("$LP$")
	end
  end
  log("\r\n")
end

end

function statusbar.init()
  local p = createPanel("bottom", 32)
  r = p:setrender(statusbar.render)
  r:setbackground(0,0,0)  
  objs.pen1 = r:createpen{ style ="solid", width = 1, r = 0, g = 0, b = 120 }
  objs.brush1 = r:createbrush{ style ="solid", r = 200, g = 0, b = 200 }
  --objs.font1 = r:createfont{ font="fixedsys", height = 11, bold = 0 }
  objs.font1 = r:defaultfont()
end

function statusbar.release()
	--saveTable({test="123", check="12345"}, getProfile()..".xml")
end
