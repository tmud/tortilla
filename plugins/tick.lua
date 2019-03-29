-- tick
-- Плагин Tortilla mud client

-- Триггеры для начала отсчета нового тика
local anchors = {
 "^Минул час.",
}
-- Количество секунд в одном тике
local seconds = 60 

local ticker = {}
function ticker.name()
  return 'Тикер с большими цифрами'
end
function ticker.description()
  local s = { 'Создает окно с таймером с большим размером цифр',
  'Меняет цвет при приближении к нулевому значению.',
  'Настройки задаются в файле плагина.'
  }
  return table.concat(s, '\r\n')
end
function ticker.version()
  return '-'
end

local ticker_text = ''
local text_width = 0
local text_height = 0

local function render()
  local x = (r:width() - text_width) / 2
  local y = (r:height() - text_height) / 2
  r:print(x, y, ticker_text)
end

local counter, start_time

local function start_new_tick()
  start_time = system.getTicks()
end

local function set_ticker(t)
  ticker_text = t
  r:update()
end

function ticker.connect()
  set_ticker('??')
end

function ticker.tick()
  if start_time then
    local time = system.getTicks()
    if time >= start_time then
      counter = math.floor( (time - start_time) / 1000 )
      counter = seconds - counter
      if counter < 0 then counter = 0; r:textColor(props.paletteColor(9))
      elseif (counter <= seconds) and (counter >= seconds/2) then r:textColor(props.paletteColor(10))
      elseif (counter < seconds/2) and (counter >= seconds/4) then r:textColor(props.paletteColor(11))
      elseif (counter < seconds/4) and (counter >= 0) then r:textColor(props.paletteColor(9))
      end
    else
      counter = 0
    end
    set_ticker(''..counter)
    if counter == 0 then
      r:textColor(props.paletteColor(0))
      start_new_tick()
    end
  else
    counter = nil
  end
end

function ticker.init()
  -- создаем окошко
  if createWindowDpi then
    w = createWindowDpi("Тик", 80, 80, true)
  else
    w = createWindow("Тик", 80, 80, true)
  end
  -- если не создалось - сообщаем пользователю
  if not w then terminate("Ошибка при создании окна") end
  -- создаем рендер для окошка
  r = w:setRender(render)
  -- задаем цвет фона по системным настройкам клиента
  r:setBackground(props.backgroundColor())
  -- цвет шрифта по умолчанию 15 белый, 9 красный
  r:textColor(props.paletteColor(15))
  -- увеличивыем размер системного шрифта
  local font = r:createFont{name=props.currentFont(), height=25}
  -- выбираем шрифт, который создали
  r:select(font)
  text_width = r:textWidth('??')
  text_height = r:fontHeight()
  start_new_tick()
  for _,v in pairs(anchors) do
    createTrigger(v, start_new_tick)
  end
end

return ticker
