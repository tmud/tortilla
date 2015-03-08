-- statusbar
-- Плагин для Tortilla mud client

statusbar = {}
function statusbar.name() 
    return 'Гистограммы здоровья и энергии'
end

function statusbar.description()
return 'Плагин отображает информацию о здоровье и энергии\r\n\z
в виде полосок на отдельной панели клиента.'
end

function statusbar.version()
    return '1.0'
end

local objs = {}
local r, regexp, regexp2, values, cfg

function statusbar.render()
  if not cfg then
    return
  end
  if regexp2 and values.hp and values.mv then
    if values.maxhp and values.maxmv then
       statusbar.drawbars()
    else
       statusbar.print("Выполните команду 'счет' для настройки плагина.")
    end
  end
  if not regexp2 and values.maxhp and values.maxmv and values.hp and values.mv then
    statusbar.drawbars()
  end
end

function statusbar.calcpercents(v, max)
  local val = tonumber(v)
  local maxval = tonumber(max)
  if not val or not maxval then
    return 0
  end
  if val <= 0 or maxval <= 0 then
    return 0
  end
  local percents = 100
  if val < maxval then
    pecents = val * 100  / maxval
  end
  return percents
end

function statusbar.drawbar(t, pos)
  local textlen = 0
  if t.text then
    r:textColor(t.color)
    textlen = r:print(pos.x, pos.y, t.text) + 2
  end
  local barlen = pos.width - textlen
  local v = (barlen * t.percents) / 100
  r:select(t.brush1)
  r:solidRect{ x=pos.x+textlen, y=pos.y, width=v, height = pos.height}
  r:select(t.brush2)
  r:solidRect{ x=pos.x+textlen+v, y=pos.y, width=barlen-v, height = pos.height}
end

function statusbar.drawbars()
  local bars = 2
  local delta_bars = 8
  local w = ((r:width()/10*3) - delta_bars*(bars-1)) / bars
  local h = r:fontHeight()
  local pos = { x=4, y=(r:height()-h)/2, width=w, height=h }

  local hpp = statusbar.calcpercents(values.hp, values.maxhp)
  statusbar.drawbar({text="HP:",percents=hpp,brush1=objs.hpbrush1,brush2=objs.hpbrush2,color=255}, pos)

  pos.x = pos.x + pos.width + delta_bars
  local mvp = statusbar.calcpercents(values.mv, values.maxmv)
  statusbar.drawbar({text="MV:",percents=mvp,brush1=objs.mvbrush1,brush2=objs.mvbrush2}, pos)
end

function statusbar.print(msg)
  local y = (r:height()-r:fontHeight()) / 2
  r:print(4, y, msg)
end

function statusbar.before(window, v)
if window ~= 0 or not cfg then return end
for i=1,v:size() do
  v:select(i)
  if v:isPrompt() then
    if regexp:findall(v:getPrompt()) then
    values.hp = regexp:get(cfg.hp)
    values.mv = regexp:get(cfg.mv)
    if not regexp2 then
      values.maxhp = regexp:get(cfg.maxhp)
      values.maxmv = regexp:get(cfg.maxmv)
    end
    r:update()
    end
  end
end
if regexp2 and v:find(regexp2) then
  values.maxhp = regexp2:get(cfg.maxhp)
  values.maxmv = regexp2:get(cfg.maxmv)
  r:update()
end
end

function statusbar.disconnect()
  values = {}
  r:update()
end

function statusbar.init()
  local file = loadTable('config.xml')
  if not file then
    return statusbar.term("Нет файла с настройками: "..getPath('config.xml'))
  end

  cfg = file.config
  local function isnumber(x) return tonumber(x) ~= nil end
  if not (isnumber(cfg.hp) and isnumber(cfg.mv) and isnumber(cfg.maxhp) and isnumber(cfg.maxmv)) then
    return statusbar.term("Ошибка в файле настроек: "..getPath('config.xml'))
  end
  if cfg.regexp then
    regexp2 = createPcre(cfg.regexp)
    if not regexp2 then
        return statusbar.term("Ошибка в регулярном выражении regexp в настройках: "..getPath('config.xml'))
    end
  end
  regexp = createPcre("[0-9]+")

  local p = createPanel("bottom", 28)
  r = p:setRender(statusbar.render)
  r:setBackground(props.backgroundColor())
  r:textColor(props.paletteColor(7))
  r:select(props.currentFont())

  objs.hpbrush1 = r:createBrush{ style ="solid", r = 255, g = 0, b = 0 }
  objs.hpbrush2 = r:createBrush{ style ="solid", r = 128, g = 0, b = 0 }
  objs.mvbrush1 = r:createBrush{ style ="solid", r = 255, g = 255, b = 0 }
  objs.mvbrush2 = r:createBrush{ style ="solid", r = 128, g = 128, b = 0 }

  --objs.pen1 = r:createPen{ style ="solid", width = 1, r = 0, g = 0, b = 120 }
  --objs.brush1 = r:createBrush{ style ="solid", r = 200, g = 0, b = 200 }
  --objs.font1 = r:createFont{ font="fixedsys", height = 11, bold = 0 }
  values = {}
end

function statusbar.term(msg)
  cfg = nil
  terminate(msg)
end
