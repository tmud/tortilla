-- bmap
-- Плагин для Tortilla mud client

local bmap = {}
function bmap.name()
  return 'Окно для автокарты Былин'
end
function bmap.description()
  return 'Плагин отображает встроенную карту Былин в отдельном окне.'
end
function bmap.version()
  return '1.0'
end

local t
local map = {}
local player_y
local last = false
local function filter(vs)
  if vs:blocks() == 0 then
    if last then
      last = false
      return false, true
    end
    return false, false
  end
  local t = vs:getBlockText(1)
  t = t:substr(1, 1)
  if t == ':' then
    last = true
    return true, true
  end
  return false, false
end

local w, r, f
local function render()
  if not player_y then return end
  local dh = r:fontHeight()
  local y = (r:height() - dh)/2
  y = y - (player_y-1)*dh
  for _,s in ipairs(map) do
    local x = 4
    for b=1,s:blocks() do
      local t = s:getBlockText(b)
      local c = s:get(b, 'textcolor')
      r:textColor(props.paletteColor(c))
      r:print(x, y, t)
      x = x + r:textWidth(t)
    end
    y = y + dh
  end
end

function bmap.init()
  t = prompt_trigger('^:.*', filter)
  w = createWindow('Автокарта Былин', 300, 300, true)
  if not w then
    terminate('Ошибка при создании окна.')
  end
  w:block('left,right,top,bottom')
  r = w:setRender(render)
  r:setBackground( props.backgroundColor() )
  r:select(props.currentFont())
end

function bmap.before(v, vd)
  if v ~= 0 then return end
  if t and t:check(vd) then
    map = t.strings
    player_y = nil
    for  i,s in ipairs(map) do
      local t = s:getText()
      if t:find('@') then player_y = i break end
    end
    r:update()
  end
end

function bmap.disconnect()
  if t then t:disconnect() end
end

function bmap.propsupdated()
  if r then 
    r:setBackground( props.backgroundColor() )
    r:select(props.currentFont())
    r:update()
  end
end

return bmap
