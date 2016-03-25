-- inveq
-- Плагин для Tortilla mud client

local inveq = {}
local colors, slots, inventory
local working = false
local delta_eq = 0
local catch_inv = false
local begin_inv, empty_inv

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
  for _,s in ipairs(slots) do
    setTextColor(colors.tegs)
    r:print(x, y, s.name..": ")
    local eq = s.equipment and s.equipment or '?'
    if eq then setTextColor(colors.equipment) r:print(x+delta_eq, y, eq) end
    y = y + h
  end
  y = y + h
  setTextColor(colors.header)
  r:print(x, y, 'Инвентарь:')
  y = y + h
  setTextColor(colors.inventory)
  for _,s in ipairs(inventory) do
    r:print(x, y, s)
    y = y + h
  end
end

local function geteq(vd)
  vd:select(1)
  local p = vd:getParameter(1)
  return p:lower()
end

local function trigger_dress(s, vd)
  local slot = slots[s]
  if not slot then return end
  local eq = geteq(vd)
  slot.equipment = eq
  for k,s in ipairs(inventory) do
    if eq == s then
      table.remove(inventory, k); break
    end
  end
  r:update()
end

local function trigger_undress(s, vd)
  local p = geteq(vd)
  for _,s in ipairs(slots) do
    if s.equipment == p then
      s.equipment = ""
      table.insert(inventory, 1, p)
      r:update()
      break
    end
  end
end

local function trigger_inventory_in(vd)
  local p = geteq(vd)
  table.insert(inventory, 1, p)
  r:update()
end

local function trigger_inventory_out(vd)
  local p = geteq(vd)
  for k,s in ipairs(inventory) do
    if p == s then
      table.remove(inventory, k); break
    end
  end
end

function inveq.before(v, vd)
  if v ~= 0 then return end
  if not catch_eq then
    if begin_inv and vd:find(begin_inv) then
      catch_eq = true
      local index,size = vd:getIndex(),vd:size()
      if index == size then return end
      vd:select(index+1)
    end
  end
  if not catch_eq then return end
  inventory = {}
  local index,size = vd:getIndex(), vd:size()
  for i=index,size do
    vd:select(i)
    if vd:isPrompt() then catch_eq = false; break end
    local item = vd:getText()
    if item ~= "" and item ~= empty_inv then
      inventory[#inventory+1] = item:lower()
    end
  end
  r:update()
end

function inveq.init()
  colors = { header = 80, tegs = 150, equipment = 180, inventory = 180 }
  local p = createPanel("right", 250)
  r = p:setRender(inveq.render)
  r:setBackground(props.backgroundColor())
  r:select(props.currentFont())
  working = false
  local t = loadTable("config.lua")
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
    -- собираем список слотов, которые будем отображать
    slots = {}
    for _,s in ipairs(t.slots) do
      if s.id then
        if not s.name then s.name = s.id end
        slots[#slots+1] = s
      end
    end
    -- Делаем мапу по ид слота для быстрого поиска
    for k,s in ipairs(slots) do
      slots[s.id] = s
    end
    -- Считаем отступ для рисования экипировки после имени слота
    local maxw = 0
    for _,s in ipairs(slots) do
      local w = r:textWidth(s.name)
      if w > maxw then maxw = w end
    end
    delta_eq = maxw + 10
    -- Создаем триггеры на одевание и раздевание
    for _,v in pairs(t.dress) do
      if not v.id or slots[v.id] then
        createTrigger(v.key, function(vd) trigger_dress(v.id, vd) end)
      end
    end
    for _,v in pairs(t.undress) do
      createTrigger(v.key, function(vd) trigger_undress(v.id, vd) end)
    end
    -- Инвентарь
    -- Триггер для начала отлова списка инвентаря
    empty_inv = t.inventory_empty
    begin_inv = nil
    if t.inventory_begin then
      begin_inv = createPcre(t.inventory_begin)
    end
    for _,v in pairs(t.inventory_in) do
      createTrigger(v, trigger_inventory_in)
    end
    for _,v in pairs(t.inventory_out) do
      createTrigger(v, trigger_inventory_out)
    end
    inventory = { "?" }
    working = true
  end
end

return inveq
