-- affects
-- Плагин для Tortilla mud client
local affects = {}
local initialized = false
local colors, good_affects, bad_affects

function affects.name()
  return 'Аффекты на персонаже'
end
function affects.description()
  return 'Плагин отображает аффекты, которые висят на персонаже.'
end
function affects.version()
  return '1.0'
end

-- рендер
local r
local function setTextColor(color)
  r:textColor(props.paletteColor(color))
end
local function update()
  r:update()
end
local function render_good_affects(x, y, h)
  for _,a in ipairs(good_affects) do
    local c = a.state and colors.good_active or colors.good_inactive
    setTextColor(c)
    r:print(x, y, a.name)
    y = y + h
  end
  return y
end
local function render_bad_affects(x, y, h)
  for _,a in ipairs(bad_affects) do
    local c = a.state and colors.bad_active or colors.bad_inactive
    setTextColor(c)
    r:print(x, y, a.name)
    y = y + h
  end
  return y
end
local function render()
  local x, y = 4, 4
  local h = r:fontHeight()
  if not initialized then
    setTextColor(colors.bad_active)
    r:print(x, y, 'Аффекты')
    y = y + h
    r:print(x, y, 'Ошибка')
    y = y + h
    r:print(x, y, 'в настройках')
    return
  end
  y = render_bad_affects(x, y, h)
  render_good_affects(x, y+h, h)
end

local function setaffect(id, state)
  local af = bad_affects[id]
  if not af then af = good_affects[id] end
  if af then
    af.state = state
  end
end

local function affects_on(t)
  if type(t) == 'string' then
    setaffect(t, true)
  else
    for _,id in ipairs(t) do
      setaffect(id, true)
    end
  end
  update()
end

local function affects_off(t)
  if type(t) == 'string' then
    setaffect(t, true)
  else
    for _,id in ipairs(t) do
      setaffect(id, false)
    end
  end
  update()
end

function affects.init()
  colors = { good_active = 10, good_inactive = 2, bad_active = 9, bad_inactive = 1 }
  local p = createPanel("right", 120)
  r = p:setRender(render)
  r:setBackground(props.backgroundColor())
  r:select(props.currentFont())
  local t = loadTable("config.lua")  
  local function istable(t) return type(t) == 'table' end
  if not istable(t) then return end
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
  -- настройка таблиц аффектов
  local function init_affects(t, st)
    for _,a in ipairs(st) do
      if a.id then
        t[#t+1] = { id = a.id, name = a.name and a.name or a.id, state = false }
      end
    end
    -- создаем мапу для поиска аффекта
    for k,a in ipairs(t) do
      t[a.id] = a
    end
  end
  good_affects = {}
  if istable(t.good_affects) then
    init_affects(good_affects, t.good_affects)
  end
  bad_affects = {}
  if istable(t.bad_affects) then
    init_affects(bad_affects, t.bad_affects)
  end
  -- создание триггеров
  for _,at in ipairs(t.triggers) do
    if at.key then
      if at.on and not at.off then
        createTrigger(at.key, function() affects_on(at.on) end)
      end
      if not at.on and at.off then
        createTrigger(at.key, function() affects_on(at.off) end)
      end
      if at.on and at.off then
        createTrigger(at.key, function() affects_on(at.on) affects_on(at.off) end)
      end
    end
  end
  initialized = true
end

return affects