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

local initialized = false
local colors, inventory, equipment

-- рендер информации экипировки и инвентаря
local render_slots_name = true
local delta_eq = 0
local r
local function setTextColor(color)
  r:textColor(props.paletteColor(color))
end
local function update()
  r:update()
end
local function render()
  local x, y = 4, 4
  local h = r:fontHeight()
  if not initialized then
    setTextColor(colors.header)
    r:print(x, y, 'Инвентарь и экипировка')
    y = y + h
    r:print(x, y, 'Ошибка в настройках плагина')
    return
  end
  setTextColor(colors.header)
  r:print(x, y, 'Экипировка')
  y = y + h
  for s in equipment.iterator() do
    if render_slots_name then
      setTextColor(colors.tegs)
      r:print(x, y, s.name..": ")
    end
    setTextColor(colors.equipment)
    r:print(x+delta_eq, y, s.equipment)
    y = y + h
  end
  y = y + h
  setTextColor(colors.header)
  r:print(x, y, 'Инвентарь')
  y = y + h
  setTextColor(colors.inventory)
  for s in inventory.iterator() do
    r:print(x, y, s)
    y = y + h
  end
end

-- база предметов для подбора именительного падежа
local db = {}
function db.load()
  if not db.objects then
    if not decl then return false end
    db.objects = decl.new()
    if not db.objects then return false end
  end
  if not db.objects:load( getPath("words.lst") ) then
    log("Не загружена таблица предметов.")
  end
  return true
end
function db.save()
  if db.objects then 
    if not db.objects:save( getPath("words.lst") ) then
      log("Не сохранена таблица предметов.")
    end
  end
end
function db.destroy()
  db.objects = nil
end
function db.add(object)
  if object and db.objects then
    db.objects:add(object)
  end
end
function db.find(object)
  if object and db.objects then
    return db.objects:find(object)
  end
end
function db.similar(ob1, ob2)
  if db.objects then
    return db.objects:compare(ob1, ob2)
  end
end

-- инвентарь
inventory = {}
function inventory.iterator()
  local i=0
  return function() i=i+1
    if not inventory.list then
      if i == 1 then return "?" end
      return
    end
    local inv = inventory.list[i]
    if inv then
      if inv.count == 1 then return inv.name end
      return inv.name.." ["..inv.count.."]"
    end
  end
end
function inventory.add(object, count)
  if not count then count = 1 end
  if not object or count < 1 then
    log("inventory.add incorrect object or count<1")
    return 
  end
  inventory.list = inventory.list or {}
  for k,v in ipairs(inventory.list) do
    if v.name == object then
      v.count = v.count + count
      return
    end
  end
  table.insert(inventory.list, 1, { name = object:lfup(), count = count })
end
function inventory.remove(object)
  if not object then
    log("inventory.remove incorrect object")
  end
  if not inventory.list then return end
  for k,s in ipairs(inventory.list) do
    if db.similar(object, s.name) then
      if s.count == 1 then
        table.remove(inventory.list, k)
      else
        s.count = s.count - 1
      end
      break
    end
  end
end
function inventory.clear()
  inventory.list = nil
end

-- экипировка
equipment = {}
function equipment.initslots(t)
  local e = {}
  for _,s in ipairs(t) do
    if s.id then
      if not s.name then s.name = s.id end
      s.equipment = "?"
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
function equipment.iterator()
  local i=0
  return function() i=i+1 return equipment.slots[i] end
end
function equipment.add(object, slot)
  if not object or not slot then
    log("equipment.add incorrect object or slot")
    return
  end
  local s = equipment.map[slot]
  if not s then
    log("equipment.add uknown slot "..slot)
    return
  end
  s.equipment = object
end
function equipment.remove(object, slot)
  if not object then
    log("equipment.remove incorrect object")
    return
  end
  if slot then
    local s = equipment.map[slot]
    if s and s.equipment~="?" and db.similar(object, eq) then s.equipment = "" end
    return
  end
  for _,s in ipairs(equipment.slots) do
    if s.equipment~="?" and db.similar(object, s.equipment) then
      s.equipment = ""
      break
    end
  end
end
function equipment.clear()
  for _,s in ipairs(equipment.slots) do
    s.equipment = "?"
  end
end
function equipment.is_slot_exist(slot)
  return equipment.map[slot] and true or false
end

-- если ip=true (именительный падеж), то добавляем в словарь
local function geteq(vd, ip)
  vd:select(1)
  local eq = vd:getParameter(1)
  if ip then
    db.add(eq)
  else
    local eqip = db.find(eq)
    if eqip then eq = eqip end
  end
  return eq:lfup()
end

-- триггер на одевание
local function trigger_dress(slot, vd, ip)
  local eq = geteq(vd, ip)
  equipment.add(eq, slot)
  inventory.remove(eq)
  update()
end

-- триггер на раздевание
local function trigger_undress(slot, vd, ip)
  local eq = geteq(vd, ip)
  equipment.remove(eq, slot)
  inventory.add(eq)
  update()
end

-- триггер на поместить в инвентарь
local function trigger_inventory_in(vd, ip)
  local eq = geteq(vd, ip)
  inventory.add(eq)
  update()
end

-- триггер на убрать из инвентаря
local function trigger_inventory_out(vd, ip)
  local eq = geteq(vd, ip)
  inventory.remove(eq)
  update()
end

-- работа с многострочными триггерами
local ml = { triggers = {} }
function ml.add(key, start_func, main_func, compare_func, ip)
  local t = ml.triggers
  t[#t+1] = { key = createPcre(key), start = start_func, main = main_func, func = compare_func(createPcre), ip = ip }
end
function ml.iterator()
  local i=0
  return function() i=i+1 return ml.triggers[i] end
end

local function trigger_equipment(slot, object)
  equipment.add(object, slot)
end

-- ловим многострочные триггеры тут
function inveq.before(v, vd)
  if v ~= 0 then return end
  if not ml.catch then
    for t in ml.iterator() do
      if vd:find(t.key) then
        ml.catch = t
        if t.start then t.start() end
        local index,size = vd:getIndex(),vd:size()
        if index == size then return end
        vd:select(index+1)
        break
      end
    end
  end
  if not ml.catch then return end
  local t = ml.catch
  local index,size = vd:getIndex(), vd:size()
  for i=index,size do
    vd:select(i)
    if vd:isPrompt() then ml.catch = nil; break end
    if not vd:isSystem() and not vd:isGameCmd() then 
      local item = vd:getText()
      if item ~= "" then
        local object, param = t.func(item)
        local number = tonumber(param)
        if number then param = number end
        if t.ip then db.add(object) end
        t.main(object, param)
      end
    end
  end
  update()
end

function inveq.init()
  initialized = false
  if not db.load() then return end
  colors = { header = 80, tegs = 150, equipment = 180, inventory = 180 }
  local p = createPanel("right", 250)
  r = p:setRender(render)
  r:setBackground(props.backgroundColor())
  r:select(props.currentFont())
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
  render_slots_name = t.slots_name and true or false
  if istable(t.slots) and istable(t.dress) and istable(t.undress) then
    -- Cобираем список слотов, которые будем отображать
    equipment.initslots(t.slots)
    -- Считаем отступ для рисования экипировки после имени слота
    delta_eq = 0
    if render_slots_name then
      local maxw = 0
      for s in equipment.iterator() do
        local w = r:textWidth(s.name)
        if w > maxw then maxw = w end
      end
      delta_eq = maxw + 10
    end
    -- Создаем триггеры на одевание и раздевание
    for _,v in pairs(t.dress) do
      if v.key and v.id then 
        if type(v.id) == 'function' then
          ml.add(v.key, nil, equipment.add, v.id, v.ip)
        else
          if equipment.is_slot_exist(v.id) then
            createTrigger(v.key, function(vd) trigger_dress(v.id, vd, v.ip) end)
          end
        end
      end
    end
    for _,v in pairs(t.undress) do
      if v.key then
        createTrigger(v.key, function(vd) trigger_undress(v.id, vd, v.ip) end)
      end
    end
    -- Триггеры на добавление в инвентарь и удаления из него
    for _,v in pairs(t.inventory_in) do
      if v.key then
        if v.id and type(v.id) == 'function' then
          local start_func = v.clear and inventory.clear or nil
          ml.add(v.key, start_func, inventory.add, v.id, v.ip)
        else
          createTrigger(v.key, function(vd) trigger_inventory_in(vd, v.ip) end)
        end
      end
    end
    for _,v in pairs(t.inventory_out) do
      if v.key then
        if v.id and type(v.id) == 'function' then
          ml.add(v.key, nil, inventory.remove, v.id, v.ip)
        else
          createTrigger(v.key, function(vd) trigger_inventory_out(vd, v.ip) end)
        end
      end
    end
    inventory.clear()
    initialized = true
  end
end

function inveq.release()
  db.save()
  db.destroy()
end

function inveq.disconnect()
  db.save()
  equipment.clear()
  inventory.clear()
  update()
end

return inveq
