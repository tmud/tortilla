-- status
-- Плагин для Tortilla mud client

-- Местоположение в окне клиента "top" или "bottom"
local position = "bottom"

-- Количество панелей. Чем больше панелей, тем меньше информации вмещает каждая панель
local count = 7

local status = {}
function status.name()
    return 'Панель статусов'
end

function status.description()
    local p=props.cmdPrefix()
    local s = 'Плагин создает панель с миниокнами, куда можно выводить дополнительную информацию.\r\n'
    s = s..'Добавляет команду '..p..'status {номер окна} {text} [{цвет}]\r\n'
    s = s..'Цвет не обязательный параметр. Он задается в формате клиента (см. справку #help color).\r\n'
    s = s..'Плагин повторяет функционал панели статусов мад-клиента jmc3.\r\n'
    s = s..'Количество миниокон: '..count..'. Количество можно изменить в файле плагина plugins/status.lua'
    return s
end

function status.version()
    return '1.01'
end

local r
local delta = 8
local panels = {}

function status.connect()
  panels = {}
  r:update()
end

local function render()
  local h = r:fontHeight()
  local w = (r:width() - delta*2 - (count-1)*delta) / count;
  local p = { x = delta, y = (r:height()-h)/2, height = h, width = w }
  for i=1,count do
    local t = panels[i]
    if t then
      p.color = t.color
      r:solidRect(p)
      r:textColor(t.tcolor)
      r:print(p, t.text)
    end
    p.x = p.x + w + delta
  end
end

local function cmdstr(t) 
  return "'"..props.cmdPrefix()..table.concat(t, " ").."'"
end

local function incparams(t)
  log("Некорретный набор параметров: "..cmdstr(t))
  return false
end

function status.syscmd(t)
  if t[1] ~= 'status' then
   return t
  end
  if #t ~= 3 and #t ~= 4 then
    return incparams(t)
  end
  local v = tonumber(t[2])
  if not v or v < 1 or v > count then
    return incparams(t)
  end
  local tcolor, color
  if #t == 4 then
    tcolor, color = translateColors(t[4])
    if not tcolor then
      return incparams(t)
    end
  else
    tcolor = props.paletteColor(7)
    color = props.backgroundColor()
  end
  panels[v] = { text = tostring(t[3]), color = color, tcolor = tcolor }
  r:update()
  return nil
end

function status.init()
  local p = createPanel(position, 28)
  r = p:setRender(render)
  r:setBackground(props.backgroundColor())
  r:textColor(props.paletteColor(7))
  r:select(props.currentFont())
  addCommand('status')
end

return status