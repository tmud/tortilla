-- statusbar
-- Плагин для Tortilla mud client

local colors = {
hp1 = {r=240},
hp2 = {r=128},
mv1 = {r=240,g=240},
mv2 = {r=128,g=128},
mn1 = {b=240},
mn2 = {b=128},
exp1 = {r=240,g=240,b=240},
exp2 = {r=128,g=128,b=128}
}

statusbar = {}
function statusbar.name() 
    return 'Гистограммы здоровья, маны, энергии, опыта'
end

function statusbar.description()
return 'Плагин отображает информацию о здоровье, мане, энергии и опыте\r\n\z
в виде полосок на отдельной панели клиента.'
end

function statusbar.version()
    return '1.0'
end

local objs = {}
local r, regnum, regmain, regexp, values, cfg
local round = math.floor

function statusbar.render()
  if not cfg then
    return
  end
  local showmsg = false
  if regexp and not values.exp then
    showmsg = true
  end
  if not showmsg and regmain then
    if not (values.maxhp and values.maxmv) then
      showmsg = true
    end
  end
  if showmsg then
    if values.hp and values.mv then
      statusbar.print("Выполните команду 'счет' для настройки плагина.")
    end
    return
  end
  statusbar.drawbars()
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
  local bars = 4
  local delta_bars = 10
  local w = round( ((r:width()/10*5) - delta_bars*(bars-1)) / bars )
  local h = r:fontHeight()
  local pos = { x=4, y=(r:height()-h)/2, width=w, height=h }

  local hpbar = {val=values.hp,maxval=values.maxhp,text="HP:",brush1=objs.hpbrush1,brush2=objs.hpbrush2,color=colors.hp1}
  if statusbar.drawbar(hpbar, pos) then
    pos.x = pos.x + pos.width + delta_bars
  end

  local mnbar = {val=values.mn, maxval=values.maxmn,text="MA:",brush1=objs.mnbrush1,brush2=objs.mnbrush2,color=colors.mn1}
  if statusbar.drawbar(mnbar, pos) then
    pos.x = pos.x + pos.width + delta_bars
  end

  local mvbar = {val=values.mv,maxval=values.maxmv,text="MV:",brush1=objs.mvbrush1,brush2=objs.mvbrush2,color=colors.mv1}
  if statusbar.drawbar(mvbar, pos) then
    pos.x = pos.x + pos.width + delta_bars
  end

  if values.dsu and values.exp then
    local expbar = {val=values.exp,maxval=values.dsu+values.exp,text="XP:",brush1=objs.expbrush1,brush2=objs.expbrush2,color=colors.exp1}
    statusbar.drawbar(expbar, pos)
  end
end

function statusbar.print(msg)
  r:textColor(props.paletteColor(7))
  local y = (r:height()-r:fontHeight()) / 2
  r:print(4, y, msg)
end

local function get(v) return v and regnum:get(v) or nil end
local function getm(v) return v and regmain:get(v) or nil end

function statusbar.before(window, v)
if window ~= 0 or not cfg then return end
for i=1,v:size() do
  v:select(i)
  if v:isPrompt() then
    if regnum:findall(v:getPrompt()) and regnum:size() > 4 then
    values.hp = get(cfg.hp)
    values.mv = get(cfg.mv)
    values.mn = get(cfg.mn)
    values.dsu = get(cfg.dsu)
    if not regmain then
      values.maxhp = get(cfg.maxhp)
      values.maxmv = get(cfg.maxmv)
      values.maxmn = get(cfg.maxmn)
    end
    r:update()
    end
  end
end
if regmain and v:find(regmain) then
  values.maxhp = getm(cfg.maxhp)
  values.maxmv = getm(cfg.maxmv)
  values.maxmn = getm(cfg.maxmn)
  r:update()
end
if regexp and v:find(regexp) then
  values.exp = regexp:get(1)
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

  if cfg.regmain then
    regmain = createPcre(cfg.regmain)
    if not regmain then
        return statusbar.term("Ошибка в регулярном выражении regmain в настройках: "..getPath('config.xml'))
    end
  end
  if cfg.regexp then
    regexp = createPcre(cfg.regexp)
    if not regexp then
        return statusbar.term("Ошибка в регулярном выражении regexp в настройках: "..getPath('config.xml'))
    end
  end
  regnum = createPcre("[0-9]+")

  local p = createPanel("bottom", 28)
  r = p:setRender(statusbar.render)
  r:setBackground(props.backgroundColor())
  r:textColor(props.paletteColor(7))
  r:select(props.currentFont())

  objs.hpbrush1 = r:createBrush{style ="solid", color=colors.hp1}
  objs.hpbrush2 = r:createBrush{style ="solid", color=colors.hp2}
  objs.mvbrush1 = r:createBrush{style ="solid", color=colors.mv1}
  objs.mvbrush2 = r:createBrush{style ="solid", color=colors.mv2}
  objs.mnbrush1 = r:createBrush{style ="solid", color=colors.mn1}
  objs.mnbrush2 = r:createBrush{style ="solid", color=colors.mn2}
  objs.expbrush1 = r:createBrush{style ="solid", color=colors.exp1}
  objs.expbrush2 = r:createBrush{style ="solid", color=colors.exp2}

  values = {}
end

function statusbar.term(msg)
  cfg = nil
  terminate(msg)
end
