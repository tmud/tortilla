-- inveq
-- Плагин для Tortilla mud client

local inveq = {}

local colors, slots, inventory, equipment
local working = false
local delta_eq = 0

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
  local h = r:fontHeight()
  if not working then
    setTextColor(colors.header)
	r:print(x, y, 'Инвентарь и экипировка')
	y = y + h
    r:print(x, y, 'Ошибка в настройках')
    return
  end
  setTextColor(colors.header)
  r:print(x, y, 'Экипировка:')
  y = y + h
  for k,s in ipairs(slots) do
    setTextColor(colors.tegs)
    r:print(x, y, s.name..": ")
    local eq = equipment[k]
    if eq then setTextColor(colors.equipment) r:print(x+delta_eq, y, eq) end
    y = y + h
  end
  y = y + h
  setTextColor(colors.header)
  r:print(x, y, 'Инвентарь:')
end

local function trigger_dress(s, vd)
end

local function trigger_undress(s, vd)
end

function inveq.init()
  colors = { header = 80, tegs = 150, equipment = 180, inventory = 180 }
  local p = createPanel("right", 250)
  r = p:setRender(inveq.render)
  r:setBackground(props.backgroundColor())
  r:select(props.currentFont())
  working = false
  local t = loadTable("test.xml")
  if not t then return end
  saveTable(t, "test2.xml")
  
  
  if istable(t.colors) then
    for k,v in pairs(t.colors) do
      local color = tonumber(v)
      if color >= 0 and color <= 255 then
        for c,_ in pairs(colors) do
          if c == k then colors[k] = color break end
        end
      end
    end
  end
  if istable(t.slots) and istable(t.dress) and istable(t.undress) then
    slots = {}
    for _,s in ipairs(t.slots) do
      if s.id then
        if not s.name then s.name = s.id end
        slots[#slots+1] = s
      end
    end
    local maxw = 0
    for _,s in ipairs(slots) do
      local w = r:textWidth(s.name)
      if w > maxw then maxw = w end
    end
    delta_eq = maxw + 20
    
    equipment = { head="Шлем", boots = "Сапоги" }
    inventory = {}
    for k,v in pairs(t.dress) do
      createTrigger(v, function(vd) trigger_dress(k, vd) end)
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
