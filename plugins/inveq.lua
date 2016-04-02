-- inveq
-- Плагин для Tortilla mud client

local inveq = {}
local colors, slots, inventory
local working = false
local delta_eq = 0
local catch_inv = false
local begin_inv, empty_inv
local decllib

function inveq.name()
  return 'Инвентарь и экипировка'
end

function inveq.description()
  return 'Плагин отображает экипировку, которая одета на персонаже и инвентарь\r\nперсонажа в отдельном окне.'
end

function inveq.version()
  return '1.0'
end

local function istable(t)
  return type(t) == 'table'
end

local r
local function setTextColor(color)
  r:textColor(props.paletteColor(color))
end
local function update()
  r:update()
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
  y = y + 8
  setTextColor(colors.header)
  r:print(x, y, 'Инвентарь:')
  y = y + h
  setTextColor(colors.inventory)
  for _,s in ipairs(inventory) do
    r:print(x, y, s)
    y = y + h
  end
end

-- если ip=true (именительный падеж), то работаем со словарем
local function geteq(vd, ip)
  vd:select(1)
  local p = vd:getParameter(1)
  if ip then
    decllib:add(p)
  else
    local eqip = decllib:find(p)
    if eqip then p = eqip end
  end
  return p:lfup()
end

local function similar(s1, s2)
  return decllib:compare(s1, s2)
end

local function trigger_dress(s, vd, ip)
  local slot = slots[s]
  if not slot then return end
  local eq = geteq(vd, ip)
  slot.equipment = eq
  for k,s in ipairs(inventory) do
    if similar(eq, s) then
      table.remove(inventory, k); break
    end
  end
  update()
end

local function trigger_undress(s, vd)
  local p = geteq(vd)
  for _,s in ipairs(slots) do
    if similar(p, s.equipment) then
      s.equipment = ""
      table.insert(inventory, 1, p)
      update()
      break
    end
  end
end

local function trigger_inventory_in(vd, ip)
  local p = geteq(vd, ip)
  table.insert(inventory, 1, p)
  update()
end

local function trigger_inventory_out(vd)
  local p = geteq(vd)
  for k,s in ipairs(inventory) do
    if similar(p, s) then
      table.remove(inventory, k)
      update()
      break
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
    if not vd:isSystem() and not vd:isGameCmd() then 
      local item = vd:getText()
      if item ~= "" and item ~= empty_inv then
        decllib:add(item)
        inventory[#inventory+1] = item:lfup()
      end
    end
  end
  update()
end

function inveq.init()
  -- модуль для работы с падежами
  decllib = decl.new()
  if not decllib then return end
  decllib:load( getPath("words.lst") )
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
  if istable(t.slots) and istable(t.eqcmd) and istable(t.dress) and istable(t.undress) then
    -- Cобираем список слотов, которые будем отображать
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
    for _,v in pairs(t.eqcmd) do
      if not v.id or slots[v.id] then
        createTrigger(v.key, function(vd) trigger_dress(v.id, vd, true) end)
      end
    end
    for _,v in pairs(t.dress) do
      if not v.id or slots[v.id] then
        createTrigger(v.key, function(vd) trigger_dress(v.id, vd) end)
      end
    end
    for _,v in pairs(t.undress) do
      createTrigger(v.key, function(vd) trigger_undress(v.id, vd) end)
    end
    -- Инвентарь
    empty_inv = t.inventory_empty
    -- Триггер для отлова списка инвентаря
    begin_inv = nil
    if t.inventory_begin then
      begin_inv = createPcre(t.inventory_begin)
    end
    -- Триггеры на добавление в инвентарь и удаления из него
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

function inveq:release()
  decllib:save( getPath("words.lst") )
  decllib = nil
end

return inveq
