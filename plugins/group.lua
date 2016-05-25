-- group
-- Плагин для Tortilla mud client
local group = {}
local initialized = false

function group.name()
  return 'Информация о группе'
end
function group.description()
  return 'Плагин отображает информацию о группе, в которой вы играете.'
end
function group.version()
  return '1.0'
end

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
    --setTextColor(colors.header)
    r:print(x, y, 'Группа')
    y = y + h
    r:print(x, y, 'Ошибка в настройках плагина')
    return
  end
  --setTextColor(colors.header)  
  r:print(x, y, 'Группа')
end

function group.init()
  initialized = false
  --colors = { header = 80, tegs = 150, equipment = 180, inventory = 180 }
  local p = createPanel("right", 250)
  r = p:setRender(render)
  r:setBackground(props.backgroundColor())
  r:select(props.currentFont())
  local t = loadTable("config.lua")
  if not t then return end
  
  initialized = true
end


return group