-- affects
-- Плагин для Tortilla mud client

local affects = {}
local colors = { header = 180 }
local working = false

function affects.name()
  return 'Аффекты на персонаже'
end

function affects.description()
  return 'Плагин отображает аффекты, которые висят на персонаже.'
end

function affects.version()
  return '1.0'
end

local function setTextColor(color)
  r:textColor(props.paletteColor(color))
end

function affects.render()
  local x, y = 4, 4
  if not working then
    setTextColor(colors.header)
    r:print(x, y, 'Ошибка в настройках')
    return
  end  
  local h = r:fontHeight()
  setTextColor(colors.header)
  r:print(x, y, 'Экипировка:')
end

function affects.init()
  local p = createPanel("right", 200)
  r = p:setRender(affects.render)
  r:setBackground(props.backgroundColor())
  r:select(props.currentFont())
end

return affects