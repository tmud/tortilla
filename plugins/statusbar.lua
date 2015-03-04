-- statusbar
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
local regexp = nil
local regexp2 = nil
local cfg = { 'hp', 'maxhp', 'vp', 'maxvp' }
local values = {}

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
  v:select(i)
  if v:isprompt() then
    if regexp:findall(v:getprompt()) then
	  for k,v in ipairs(cfg) do
		values[v] = regexp:get(k-1)
	  end
	end
  end
end
if v:find(regexp2) then
  log('maxhp='..regexp2:get(1)..',maxvp='..regexp2:get(2)..'\r\n')  
end
--[[for k,v in pairs(values) do
  log(k..'='..v..'\r\n')
end]]
end

function statusbar.init()
  local p = createPanel("bottom", 28)
  r = p:setrender(statusbar.render)
  r:setbackground(0,0,0)  
  objs.pen1 = r:createpen{ style ="solid", width = 1, r = 0, g = 0, b = 120 }
  objs.brush1 = r:createbrush{ style ="solid", r = 200, g = 0, b = 200 }
  --objs.font1 = r:createfont{ font="fixedsys", height = 11, bold = 0 }
  objs.font1 = r:defaultfont()
  regexp = createPcre("[0-9]+")
  regexp2 = createPcre("^Вы имеете [0-9]+\\(([0-9]+)\\) единиц здоровья, [0-9]+\\(([0-9]+)\\) энергетических единиц.")
end

function statusbar.release()
	--saveTable({prompt="([0-9])ж/([0-9])Ж"}, "config.xml")
end
