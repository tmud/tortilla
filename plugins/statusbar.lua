-- statusbar
-- Плагин для Tortilla mud client

statusbar = {}
function statusbar.name() 
    return 'Гистограммы здоровья, маны и энергии'
end

function statusbar.description()
return 'Плагин отображает информацию о здоровье, мане и энергии\r\n\z
в виде полосок на отдельной панели клиента.'
end

function statusbar.version()
    return '1.0'
end

local objs = {}
local r, regexp, regexp2, values, cfg
local round = math.floor

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

function statusbar.drawbar(t, pos)
  local val = tonumber(t.val)
  local maxval = tonumber(t.maxval)
  if not val or not maxval then return false end

  local percents = 0
  if val > 0 and maxval > 0 then
    if val < maxval then 
    percents = val * 100 / maxval
    else
      percents = 100
    end
  end

  local textlen = 0
  if t.text then
    r:textColor(t.color)
    textlen = r:print(pos.x, pos.y, t.text) + 2
  end
  local barlen = pos.width - textlen
  local v = round( (barlen * percents) / 100 )
  r:select(t.brush1)
  r:solidRect{x=pos.x+textlen, y=pos.y, width=v-1, height = pos.height}
  r:select(t.brush2)
  r:solidRect{x=pos.x+textlen+v, y=pos.y, width=barlen-v, height = pos.height}
  return true
end

function statusbar.drawbars()
  local bars = 3
  local delta_bars = 8
  local w = round( ((r:width()/10*4) - delta_bars*(bars-1)) / bars )
  local h = r:fontHeight()
  local pos = { x=4, y=(r:height()-h)/2, width=w, height=h }

  local hpbar = {val=values.hp, maxval=values.maxhp,text="HP:",brush1=objs.hpbrush1,brush2=objs.hpbrush2,color={r=255}}
  if statusbar.drawbar(hpbar, pos) then
    pos.x = pos.x + pos.width + delta_bars
  end

  local mnbar = {val=values.mn, maxval=values.maxmn,text="MA:",brush1=objs.mnbrush1,brush2=objs.mnbrush2,color={b=255}}
  if statusbar.drawbar(mnbar, pos) then
    pos.x = pos.x + pos.width + delta_bars
  end

  local mvbar = {val=values.mv, maxval=values.maxmv,text="MV:",brush1=objs.mvbrush1,brush2=objs.mvbrush2,color={r=255,g=255}}
  statusbar.drawbar(mvbar, pos)
end

function statusbar.print(msg)
  r:textColor(props.paletteColor(7))
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
    values.mn = regexp:get(cfg.mn)
    if not regexp2 then
      values.maxhp = regexp:get(cfg.maxhp)
      values.maxmv = regexp:get(cfg.maxmv)
      values.maxmn = regexp:get(cfg.maxmn)
    end
    r:update()
    end
  end
end
if regexp2 and v:find(regexp2) then
  values.maxhp = regexp2:get(cfg.maxhp)
  values.maxmv = regexp2:get(cfg.maxmv)
  values.maxmn = regexp2:get(cfg.maxmn)
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

  objs.hpbrush1 = r:createBrush{style ="solid", color={r=255}}
  objs.hpbrush2 = r:createBrush{style ="solid", color={r=128}}
  objs.mvbrush1 = r:createBrush{style ="solid", color={r=255,g=255}}
  objs.mvbrush2 = r:createBrush{style ="solid", color={r=128,g=128}}
  objs.mnbrush1 = r:createBrush{style ="solid", color={b=255}}
  objs.mnbrush2 = r:createBrush{style ="solid", color={b=128}}

  values = {}
end

function statusbar.term(msg)
  cfg = nil
  terminate(msg)
end
