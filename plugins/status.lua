-- status
-- Плагин для Tortilla mud client

local status = {}
function status.name()
  return 'Панель статусов'
end

local count = 0
function status.description()
  local p=props.cmdPrefix()
  local s = { 'Плагин создает панель с миниокнами, куда можно выводить дополнительную информацию.',
  'Добавляет команду '..p..'status {номер окна} {text} [{цвет}]',
  'Цвет - это не обязательный параметр. Он задается в формате клиента (см. справку #help color).',
  'Плагин повторяет функционал панели статусов мад-клиента jmc3.',
  'Он также умеет отчитывать время между тиками. Количество миниокон: '..count..'.',
  'Настройки задаются в файле: '..getPath('config.lua') }
  return table.concat(s, '\r\n')
end

function status.version()
  return '1.04'
end

local r
local delta = 8
local panels = {}

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
  local tcolor = props.paletteColor(7)
  local color = props.backgroundColor()
  if #t == 4 then
    tcolor, color = translateColors(t[4], tcolor, color)
    if not tcolor then
      return incparams(t)
    end
  end
  panels[v] = { text = tostring(t[3]), color = color, tcolor = tcolor }
  r:update()
  return nil
end

local ticker_on, ticker_window, ticker_seconds
local ticker_panel = { text = '', tcolor = props.paletteColor(7), color = props.backgroundColor() }

local function set_ticker(t)
  if ticker_on then
    ticker_panel.text = t
    panels[ticker_window] = ticker_panel
    r:update()
  end
end

local counter, start_time

local function start_new_tick()
  start_time = system.getTicks()
end

function status.tick()
  if start_time then
    local time = system.getTicks()
    if time >= start_time then
      counter = math.floor( (time - start_time) / 1000 )
      counter = ticker_seconds - counter
      if counter < 0 then counter = 0 end
    else
      counter = 0
    end
    set_ticker(''..counter)
  else
    counter = nil
  end
end

function status.connect()
  panels = {}
  set_ticker('ticker')
end

function status.disconnect()
  set_ticker('')
end

function status.init()
  local function term(t) terminate("Некорректый параметр '"..t.."', "..getPath('config.lua')) end
  ticker_on = false
  local t = loadTable('config.lua')
  if not t then terminate('Нет файла настроек '..getPath('config.lua')..'.') end
  if not t.position or (t.position ~= 'top' and t.position ~= 'bottom') then term('position') end
  if not t.count or type(t.count) ~= 'number' or t.count < 1 then term('count') end
  if t.ticker then
    if type(t.ticker) ~= 'string' or t.ticker == '' then term('ticker') end
    if not t.ticker_seconds or type(t.ticker_seconds) ~= 'number' or t.ticker_seconds < 1 then term('ticker_seconds') end
    if not t.ticker_window or type(t.ticker_window) ~= 'number' or t.ticker_window < 1 or t.ticker_window > t.count then term('ticker_window') end
    if t.ticker_color and type(t.ticker_color) ~= 'string' then term('ticker_color') end
    if not createTrigger(t.ticker, start_new_tick) then term('ticker') end
    ticker_seconds = t.ticker_seconds
    ticker_window = t.ticker_window
    ticker_panel.tcolor, ticker_panel.color = translateColors(t.ticker_color, ticker_panel.tcolor, ticker_panel.color)
    ticker_on = true
  end
  count = t.count
  local font = props.currentFontHeight()+2
  local p = createPanel(t.position, font)
  r = p:setRender(render)
  r:setBackground(props.backgroundColor())
  r:textColor(props.paletteColor(7))
  r:select(props.currentFont())
  addCommand('status')
  set_ticker('ticker')
end

return status
