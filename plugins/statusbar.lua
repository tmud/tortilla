-- statusbar
-- Плагин для Tortilla mud client

-- Местоположение в окне клиента "top" или "bottom"
local position = "bottom"

-- Цвета баров hp(здоровья),mv(энергия),mn(мана),exp(опыт)
local colors = {
hp1 = {r=240},
hp2 = {r=128},
mv1 = {r=240,g=240},
mv2 = {r=128,g=128},
mn1 = {g=128,b=255},
mn2 = {g=64,b=128},
exp1 = {r=240,g=240,b=240},
exp2 = {r=128,g=128,b=128}
}

statusbar = {}
function statusbar.name() 
    return 'Гистограммы здоровья, маны, энергии, опыта'
end

function statusbar.description()
return 'Плагин отображает информацию о здоровье, мане, энергии и опыте\r\n\z
в виде полосок на отдельной панели клиента. Требует настройки. Про настройку\r\n\z
читайте в справке к клиенту (#help plugin_statusbar).'
end

function statusbar.version()
    return '1.01'
end

local objs = {}
local regs = {}
local bars = 0
local connect = false
local tegs = { 'hp','mn','mv','xp','dsu' }

local r, regnum, values, cfg
local round = math.floor

function statusbar.render()
  if not cfg or not connect or bars == 0 then
    return
  end

  local showmsg = false
  if values.hp and values.mv then
    if not values.maxhp or not values.maxmv then
        showmsg = true
    end
  end

  if showmsg then
    statusbar.print("Выполните команду 'счет' для настройки плагина.")
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
  local delta_bars = 10
  local w = round( ((r:width()/10*6) - delta_bars*(bars-1)) / bars )
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

  local val, maxval
  if values.xp and values.dsu then
    val = values.xp
    maxval = values.xp + values.dsu
  elseif values.xp and values.summ then
    val = values.xp
    maxval = values.summ
  elseif values.dsu and values.summ then
    val = values.summ - values.dsu
    maxval = values.summ
  elseif values.maxdsu and values.maxxp then
    val = values.maxxp
    maxval = values.summ
  end
  local expbar = {val=val,maxval=maxval,text="XP:",brush1=objs.expbrush1,brush2=objs.expbrush2,color=colors.exp1}
  statusbar.drawbar(expbar, pos)
end

function statusbar.print(msg)
  r:textColor(props.paletteColor(7))
  local y = (r:height()-r:fontHeight()) / 2
  r:print(4, y, msg)
end

function statusbar.before(window, v)
if window ~= 0 or not cfg then return end
  local update = false
  for i=1,v:size() do
    v:select(i)
    if v:isPrompt() and regnum:findall(v:getPrompt()) and regnum:size()-1 > 2 then
      for _,teg in pairs(tegs) do
         local c = cfg[teg]
         if c and c.prompt then
           values[teg] = tonumber(regnum:get(c.prompt))
         end
      end
      update = true
    end
  end
  for id,regexp in pairs(regs) do
    if v:find(regexp) then
      for _,teg in pairs(tegs) do
        local c = cfg[teg]
        if c and c.regid == id then
          values['max'..teg] = tonumber(regexp:get(c.regindex))
          update = true
        end
      end
    end
  end
  local mxp = tonumber(values.maxxp)
  local mdsu = tonumber(values.maxdsu)
  if mxp and mdsu then
    values.summ = mxp + mdsu
  end
  if update then
    r:update()
  end
end

function statusbar.connect()
  connect = true
end

function statusbar.disconnect()
  connect = false
  values = {}
  r:update()
end

local function readcfg(teg)
  local score_index = tonumber(cfg.maxparams[teg])
  local prompt_index= tonumber(cfg.baseparams[teg])
  if score_index or prompt_index then
    local id = nil
    if score_index then
      id = cfg.maxparams[teg..'id']
      if not id then
        return false, 'Не указан индекс регулярного выражения '..teg..'id'
      end
      if not cfg.regexp[id] then
        return false, 'Нет регулярного выражения для '..id
      end
      if not regs[id] then
         regs[id] = createPcre(cfg.regexp[id])
         if not regs[id] then
           return false, 'Ошибка в регулярном выражении '..id
         end
      end
    end
    local c = {}
    c.regindex = score_index
    c.prompt = prompt_index
    c.regid = id
    cfg[teg] = c
    return true
  end
  return false
end

function statusbar.init()
  local file = loadTable('config.xml')
  if not file then
    cfg = nil
    return terminate("Нет файла с настройками: "..getPath('config.xml'))
  end
  cfg = file
  bars = 0
  regs = {}
  local msgs = {}
  for _,v in pairs(tegs) do
    local res,msg = readcfg(v)
    if not res and msg then
      msgs[msg] = true
      msgs.notempty = true
    end
    if res then
      bars = bars + 1
    end
  end
  if msgs.notempty then
    msgs.notempty = nil
    log('Ошибки в файле настрек: '..getPath('config.xml'))
    for k,_ in pairs(msgs) do
      log(k)
    end
  end

  regnum = createPcre("[0-9]+")

  local p = createPanel(position, 28)
  r = p:setRender(statusbar.render)
  r:setBackground(props.backgroundColor())
  r:textColor(props.paletteColor(7))
  r:select(props.currentFont())

  objs.hpbrush1 = r:createBrush{color=colors.hp1}
  objs.hpbrush2 = r:createBrush{color=colors.hp2}
  objs.mvbrush1 = r:createBrush{color=colors.mv1}
  objs.mvbrush2 = r:createBrush{color=colors.mv2}
  objs.mnbrush1 = r:createBrush{color=colors.mn1}
  objs.mnbrush2 = r:createBrush{color=colors.mn2}
  objs.expbrush1 = r:createBrush{color=colors.exp1}
  objs.expbrush2 = r:createBrush{color=colors.exp2}

  values = {}
  connect = props.connected()
end
