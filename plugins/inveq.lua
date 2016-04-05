-- inveq
-- Плагин для Tortilla mud client

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

-- база предметов для подбора именительного падежа
local db = {}
function db.load()
  if not db.objects then
    if not decl then return false end
    db.objects = decl.new()
    if not db.objects then return false end
  end
  db.objects:load( getPath("words.lst") )
  return true
end
function db.save()
  if db.objects then 
    db.objects:save( getPath("words.lst") )
  end
end
function db.destroy()
  db.objects = nil
end
function db.add(object)
  if db.objects then
    db.objects:add(object)
  end
end
function db.find(object)
  if db.objects then
    return db.objects:find(object)
  end
end
function db.similar(ob1, ob2)
  if db.objects then
    return db.objects:compare(s1, s2)
  end
end

-- инвентарь
local inventory = {}
function inventory.get()
  return inventory.list and inventory.list or{ "?" }
end
function inventory.add(object, ip)
  if ip then db.add(object) end
  inventory.list = inventory.list or {}
  table.insert(inventory.list, 1, object:lfup())
end
function inventory.remove(object)
  if not inventory.list then return end
  for k,s in ipairs(inventory.list) do
    if db.similar(object, s) then
      table.remove(inventory.list, k); break
    end
  end
end
function inventory.clear()
  inventory.list = nil
end

-- экипировка
local equipment = {}
function equipment.addslots(t)
  local e = {}
  for _,s in ipairs(t) do
    if s.id then
      if not s.name then s.name = s.id end
      e[#e+1] = s
    end
  end
  equipment.slots = e
  -- Делаем мапу по ид слота для быстрого поиска
  local m = {}
  for k,s in ipairs(e) do
    m[s.id] = s
  end
  equipment.map = m
end
-- Считаем отступ для рисования экипировки после имени слота
function equipment.getwidth()
  if not equipment.slots then return 0 end
  local maxw = 0
  for _,s in ipairs(equipment.slots) do
    local w = r:textWidth(s.name)
    if w > maxw then maxw = w end
  end
  return maxw
end
function equipment.iterator()
  local i=0
  return function() i=i+1 return equipment.slots[i] end
end
function equipment.add(slot, object, ip)
end
function equipment.remove(object)
  for _,s in ipairs(equipment.slots) do
    if db.similar(p, s.equipment) then
      s.equipment = ""
      
    end
  end
end




local colors
local working = false
local delta_eq = 0
local catch_inv = false
local begin_inv, empty_inv
local decllib

-- рендер информации экипировки и инвентаря
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
  --[[for _,s in ipairs(slots) do
    setTextColor(colors.tegs)
    r:print(x, y, s.name..": ")
    local eq = s.equipment and s.equipment or '?'
    if eq then setTextColor(colors.equipment) r:print(x+delta_eq, y, eq) end
    y = y + h
  end
  y = y + h]]
  setTextColor(colors.header)
  r:print(x, y, 'Инвентарь:')
  y = y + h
  setTextColor(colors.inventory)
  for _,s in ipairs(inventory.get()) do
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

-- триггер на одевание
--[[local function trigger_dress(s, vd, ip)
  local slot = slots[s]
  if not slot then return end
  local eq = geteq(vd, ip)

  equipment.add(slot, eq, ip)
  inventory.remove(eq)
  update()
end

-- триггер на раздевание
local function trigger_undress(s, vd)
  local p = geteq(vd)
  for _,s in ipairs(slots) do
    if similar(p, s.equipment) then
      s.equipment = ""
      inventory.add(p)
      update()
      break
    end
  end
end

-- триггер на поместить в инвентарь
local function trigger_inventory_in(vd, ip)
  local p = geteq(vd, ip)
  inventory.add(p)
  update()
end

-- триггер на убрать из инвентаря
local function trigger_inventory_out(vd)
  local p = geteq(vd)
  inventory.remove(p)
  update()
end

-- ловим команду инвентарь
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
  
  inventory.clear()
  local index,size = vd:getIndex(), vd:size()
  for i=index,size do
    vd:select(i)
    if vd:isPrompt() then catch_eq = false; break end
    if not vd:isSystem() and not vd:isGameCmd() then 
      local item = vd:getText()
      if item ~= "" and item ~= empty_inv then
        inventory.add(item, true)
      end
    end
  end
  update()
end
]]

function inveq.init()
  if not db.load() then return end
  colors = { header = 80, tegs = 150, equipment = 180, inventory = 180 }
  local p = createPanel("right", 250)
  r = p:setRender(inveq.render)
  r:setBackground(props.backgroundColor())
  r:select(props.currentFont())
  working = false
  local t = loadTable("config.lua")
  if not t then return end
  local function istable(t) return type(t) == 'table' end
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
    equipment.addslots(t.slots)
    -- Считаем отступ для рисования экипировки после имени слота
    delta_eq = equipment.getwidth() + 10
    
--[[
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
    inventory.clear()
    working = true]]
  end
end

function inveq.release()
--  db.save()
--  db.destroy()
end

function inveq.disconnect()
--[[  db.save()
  for _,s in ipairs(slots) do
      s.equipment = nil
  end
  inventory.clear()
  update() ]]
end

return inveq
