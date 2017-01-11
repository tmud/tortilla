-- statusbar
-- Плагин для Tortilla mud client

-- Включите режим га/автозавершения в клиенте, либо настройте распознавание prompt-строки!
-- подробнее #help plugin_statusbar
-- FAQ: Если необновляются бары, например в бою, проверьте режим га/автозавершения!

-- Местоположение в окне клиента "top" или "bottom"
local position = "bottom"

-- Цвета баров hp(здоровья),mv(энергия),mn(мана),exp(опыт)
local colors = {
hp1 = {r=240},
hp2 = {r=128},
mv1 = {r=240,g=240},
mv2 = {r=128,g=128},
mn1 = {g=128,b=240},
mn2 = {g=64,b=128},
exp1 = {r=240,g=240,b=240},
exp2 = {r=128,g=128,b=128}
}

local statusbar = {}
function statusbar.name()
    return 'Гистограммы здоровья, маны, энергии, опыта'
end

function statusbar.description()
  local s = { 'Плагин отображает информацию о здоровье, мане, энергии и опыте в виде полосок',
  'на отдельной панели клиента. Требует для работы режим га/автозавершения в маде.',
  'Требует файл с настройками. Про настройку читайте в справке к клиенту (#help statusbar).',
  'Плагин также поддерживает систему ремортов, он отслеживает команду уровни(если актуально).' }
  return table.concat(s, '\r\n')
end

function statusbar.version()
    return '1.10'
end

local objs = {}
local regs = {}
local bars = 0
local connect = false
local reinit = false
local tegs = { 'hp','mn','mv','xp','dsu','xpv' }

local r, values, cfg
local round = math.floor
local scorecmd

function statusbar.render()
  if not cfg or not connect or bars == 0 then
    return
  end
  local showmsg = false
  if values.hp and values.mv then
    if not values.maxhp or not values.maxmv then
      showmsg = true
    else
      reinit = false
      if values.hp > values.maxhp or values.mv > values.maxmv then
        reinit = true
      end
      if values.mn and values.maxmn and values.mn > values.maxmn then
        reinit = true
      end
    end
  end
  if showmsg then
    statusbar.print(4, "Выполните команду '"..scorecmd.."' для настройки плагина.")
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

  if values.xpv and values.xpm then
    local expbar = {val=values.xpv,maxval=values.xpm,text="XP:",brush1=objs.expbrush1,brush2=objs.expbrush2,color=colors.exp1}
    if statusbar.drawbar(expbar, pos) then
      pos.x = pos.x + pos.width + delta_bars
    end
  end

-- hp > maxhp or mv > maxmv or mn > maxmn (level up, affects? - неверной значение max параметров)
  if reinit then
    statusbar.print(pos.x, "("..scorecmd..")")
  end
end

function statusbar.print(x, msg)
  r:textColor(props.paletteColor(7))
  local y = (r:height()-r:fontHeight()) / 2
  r:print(x, y, msg)
end

function statusbar.xplimits()
  local lsize = #cfg.levels
  local m = values.maxxp
  local found = false
  for i=1,lsize-1 do
    local min = tonumber(cfg.levels[i])
    local max = tonumber(cfg.levels[i+1])
    if m >= min and m <= max then
      found = true
      values.minxp = min
      values.maxxp = max
      break
    end
  end
  if not found then
    values.minxp = nil
    values.maxxp = nil
  end 
end

local levels_trigger, levels_string_filter, levels_string_sep, level_prompt_filter_pcre
local function levels_prompt_filter(s)
  if level_prompt_filter_pcre:find(s) then
    return true
  end
  return false
end
local function levels_filter(vss)
  if levels_string_filter and levels_string_filter:find(vss:getText()) then
    return true,false
  end
  return false, false
end
local function collect_levels(t)
  local newlevels = {}
  for _,s in ipairs(t) do
    levels_string_filter:find(s:getText())
    local s = levels_string_filter:get(1)
    if type(s) == 'string' and s:len() > 0 then
      s = table.concat( s:tokenize(levels_string_sep) )
      s = tonumber(s)
      if not s then newlevels = nil break end
      newlevels[#newlevels+1] = s+999
    end
  end
  if #newlevels > 0 then
    local config = loadTable('config.xml')
    if not config then
      log("Не прочитался файл с настройками: "..getPath('config.xml'))
      return
    end
    if type(config.levels) ~= 'table' then
      log("В настройках нет раздела уровней(levels), уровни не обновлены.")
      return
    end
    if #config.levels ~= #newlevels then
      log("В настройках количество уровней не совпадает с игрой, уровни не обновлены.")
      return
    end
    for i,v in ipairs(newlevels) do
      config.levels[i] = v
    end
    if saveTable(config, 'config.xml') then
      reinit =  true
      for i,v in ipairs(newlevels) do
        cfg.levels[i] = v
      end
    else
      log("Не сохранился файл настроек, уровни не обновлены. Файл: "..getPath('config.xml'))
    end
  end
end

function statusbar.before(window, v)
  if window ~= 0 or not cfg then return end

  if levels_trigger and levels_trigger:check(v) then
    collect_levels( levels_trigger.strings )
  end

  local update = false
  for i=1,v:size() do
    v:select(i)
    if v:isPrompt() then
      local tmp,count = {},0
      local prompt = v:getPrompt()
      for _,teg in pairs(tegs) do
         local c = cfg[teg]
         if c and c.prompt and c.prompt:find(prompt) then
           tmp[teg] = tonumber(c.prompt:get(1))
           count = count + 1
         end
      end
      if count > 1 then
        update = true
        for k,v in pairs(tmp) do 
          values[k] = v
        end
      end
    end
  end
  for id,regexp in pairs(regs) do
    if v:find(regexp) then
      for _,teg in pairs(tegs) do
        local c = cfg[teg]
        if c and c.regid == id then
          update = true
          values['max'..teg] = tonumber(regexp:get(c.regindex))
          if teg == 'xp' and cfg.levels then
            statusbar.xplimits()
          end
          if teg == 'xpv' then
            values.maxxp = values.maxxpv
            values.maxdsu = values.dsu
          end
        end
      end
    end
  end

  values.xpv = nil
  if values.minxp and values.maxxp then
    values.xpm = values.maxxp - values.minxp
    if values.xp then
      values.xpv = values.xp-values.minxp
    elseif values.dsu then
      values.xpv = values.maxxp - values.minxp - values.dsu 
    end
  else
    if values.maxxp and values.maxdsu then
      if values.xp or values.dsu then
        values.xpm = values.maxxp + values.maxdsu
        if values.xp then
          values.xpv = values.xp
        else
          values.xpv = values.xpm - values.dsu
        end
      end
    else
      if values.xp and values.dsu then
        values.xpv = values.xp
        values.xpm = values.xp + values.dsu
      end
    end
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

function statusbar.propsupdated()
  r:setBackground(props.backgroundColor())
  r:update()
end

local function initprompt(teg)
  local prompt_letter = cfg.baseparams[teg]
  if prompt_letter then
    local c = cfg[teg] or {}
    c.prompt = createPcre("([0-9]+)"..prompt_letter)
    if not c.prompt then
      return false, 'Ошибка в параметре baseparams/'..teg
    end
    cfg[teg] = c
    return true
  end
  return false
end

local function initmax(teg)
  local score_index = tonumber(cfg.maxparams[teg])
  if score_index then
    local id = cfg.maxparams[teg..'id']
    if not id then
      return false, 'Не указан индекс регулярного выражения maxparams/'..teg..'id'
    end
    if not cfg.regexp[id] then
      return false, 'Нет регулярного выражения regexp/'..id
    end
    if not regs[id] then
       regs[id] = createPcre(cfg.regexp[id])
       if not regs[id] then
         return false, 'Ошибка в регулярном выражении regexp/'..id
       end
    end
    local c = cfg[teg] or {}
    c.regindex = score_index
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
    local res,msg = initprompt(v)
    if not res and msg then
      msgs[#msgs+1] = msg
    end
    res, msg = initmax(v)
    if not res and msg then
      msgs[#msgs+1] = msg
    end
    if res then
      bars = bars + 1
    end
  end

  if #msgs ~= 0 then
    log('Ошибки в файле настроек: '..getPath('config.xml'))
    for _,v in ipairs(msgs) do
      log(v)
    end
    return terminate("Продолжение работы невозможно")
  end

  if cfg.levels and cfg.levregexp then
    local r = cfg.levregexp
    if type(r) == 'table' and r.key and r.level then
      if r.separators then
        levels_string_sep = r.separators
      else
        levels_string_sep = ''
      end
      level_prompt_filter_pcre = nil
      if r.skipprompt then
        level_prompt_filter_pcre = createPcre(r.skipprompt)
        if not level_prompt_filter_pcre then
          terminate('Настройка обновления уровней опыта levregexp.skipprompt задана неверно.')
        end
      end
      levels_trigger = prompt_trigger(r.key, levels_filter, level_prompt_filter_pcre and levels_prompt_filter or nil)
      if not levels_trigger then
        terminate('Настройка обновления уровней опыта levregexp.key задана неверно.')
      end
      levels_string_filter = createPcre(r.level)
      if not levels_string_filter then
        terminate('Настройка обновления уровней опыта levregexp.level задана неверно.')
      end
    end
  end

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

  scorecmd = 'счет'
  if type(cfg.cmd) == 'string' then scorecmd = cfg.cmd end
end

return statusbar