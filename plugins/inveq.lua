-- inveq
-- Плагин для Tortilla mud client

local header_color = 80
local tegs_color = 150
local equipment_color = 180
local inventory_color = 180


local slots, inventory, equipment
local working = false
local delta_eq = 0

local inveq = {}
function inveq.name() 
  return 'Инвентарь и экипировка'
end

function inveq.description()
  return 'Плагин отображает экипировку, которая одета на персонаже и инвентарь\r\nперсонажа в отдельном окне.'
end

function inveq.version()
  return '1.0'
end

local r
local function setTextColor(color)
  r:textColor(props.paletteColor(color))
end
local function istable(t)
  return type(t) == 'table'
end

function inveq.render()
  local x, y = 4, 4
  if not working then
    setTextColor(header_color)
    r:print(x, y, 'Ошибка в настройках')
    return
  end  
  local h = r:fontHeight()
  setTextColor(header_color)
  r:print(x, y, 'Экипировка:')
  y = y + h
  for k,v in pairs(slots) do
    setTextColor(tegs_color)
    r:print(x, y, v..": ")
    local eq = equipment[k]
    if eq then setTextColor(equipment_color) r:print(x+delta_eq, y, eq) end
    y = y + h
  end
  y = y + h
  setTextColor(header_color)
  r:print(x, y, 'Инвентарь:')
end

local function trigger_dress(s, t)
end

local function trigger_undress(s, t)
end

function inveq.init()
  local p = createPanel("right", 300)
  r = p:setRender(inveq.render)
  r:setBackground(props.backgroundColor())
  r:select(props.currentFont())
  local t = loadTable("config.xml")
  working = false
  if istable(t.slots) and istable(t.dress) and istable(t.undress) then
    slots = t.slots
    local maxw = 0
    for _,v in pairs(slots) do
      local w = r:textWidth(v)
      if w > maxw then maxw = w end
    end
    delta_eq = maxw + 20
    
    equipment = { head="Шлем", boots = "Сапоги" }
    inventory = {}
    for k,v in pairs(t.dress) do
      
    end
    for k,v in pairs(t.undress) do
      
    end
    working = true
  end
end

function inveq.syscmd(t)
  return t
end

return inveq
