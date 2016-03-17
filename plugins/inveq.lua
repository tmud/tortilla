-- inveq
-- Плагин для Tortilla mud client

local inveq = {}

local colors, slots, names, inventory, equipment
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
  if not working then
    setTextColor(colors.header)
    r:print(x, y, 'Ошибка в настройках')
    return
  end  
  local h = r:fontHeight()
  setTextColor(colors.header)
  r:print(x, y, 'Экипировка:')
  y = y + h
  for k,v in ipairs(slots) do
    setTextColor(colors.tegs)
    r:print(x, y, names[v]..": ")
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
  local t = loadTable("config.xml")
  if not t then return end
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
    slots = t.slots
    names = {}
    if istable(t.names) then names = t.names end
    local maxw = 0
    for _,v in ipairs(slots) do
      local text = names[v]
      if not text then text = v names[v] = v end
      local w = r:textWidth(text)
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
