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
local regs = {}
local bars = 0
local tegs = { 'hp','mn','mv','xp' }

local r, regnum, values, cfg
local round = math.floor

function statusbar.render()
  if not cfg or bars == 0 then
    return
  end

  local showmsg = false
  if cfg.hp and not values.maxhp then
    showmsg = true
  end
  if values.hp and values.maxhp and values.hp > values.maxhp then
    showmsg = true
  end
  if cfg.mv and not values.maxmv then
    showmsg = true
  end   
  if values.mv and values.maxmv and values.mv > values.maxmv then
    showmsg = true
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

  if values.xp and values.maxxp then
    local summ = values.xp + values.maxxp
	local v = values.maxxp
	if cfg.extra and cfg.extra.invertxp then
	  v = values.xp
	end	
    local expbar = {val=values.xp,maxval=summ,text="XP:",brush1=objs.expbrush1,brush2=objs.expbrush2,color=colors.exp1}
    statusbar.drawbar(expbar, pos)
  end
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
         if c then
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
  if update then
    r:update()
  end
end

function statusbar.disconnect()
  values = {}
  r:update()
end

local function readcfg(teg)
  local id = cfg.maxparams[teg..'id']
  if id then
    local score_index = tonumber(cfg.maxparams[teg])
    local prompt_index= tonumber(cfg.baseparams[teg])
    if score_index and prompt_index then
      if not regs[id] then
        regs[id] = createPcre(cfg.regexp[id])
        if not regs[id] then
          log('Ошибка в регулярном выражении '..id..' в настройках: '..getPath('config.xml'))
        end
      end
      if regs[id] then
        local c = {}
        c.regindex = score_index
        c.prompt = prompt_index
        c.regid = id
        cfg[teg] = c
        return true
      end
    end
  end
  return false
end

function statusbar.init()
  local file = loadTable('config.xml')
  if not file then
    return statusbar.term("Нет файла с настройками: "..getPath('config.xml'))
  end
  cfg = file
  bars = 0
  regs = {}
  for _,v in pairs(tegs) do
    if readcfg(v) then
      bars = bars + 1
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
