-- statusbar
-- Плагин для Tortilla mud client

statusbar = {}
function statusbar.name() 
    return 'Гистограммы здоровья и энергии и др.'
end

function statusbar.description()
return 'Плагин отображает информацию о здоровье, энергии и других параметрах\r\n\z
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

function statusbar.drawbars()
  --r:select(objs.pen1)
  --r:select(objs.brush1)
  --r:rect{left = 10, right = 30, top = 10, bottom = 30}
  --r:rect{60, 10, 90, 27}
  --r:solidrect{10, 4, 160, 26}
   statusbar.print(values.hp..'/'..values.maxhp..'HP '..values.mv..'/'..values.maxmv..'MV')
end

function statusbar.print(msg)
  r:select(objs.font1)
  local y = (r:height()-r:fontheight()) / 2
  r:print(4, y, msg)
end

function statusbar.before(window, v)
if window ~= 0 or not cfg then return end
for i=1,v:size() do
  v:select(i)
  if v:isprompt() then
    if regexp:findall(v:getprompt()) then
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

  local p = createPanel("bottom", 28)
  r = p:setrender(statusbar.render)
  r:setbackground(props.backgroundColor())
  r:textcolor(props.paletteColor(7))
  objs.font1 = props.currentFont()

  --objs.pen1 = r:createpen{ style ="solid", width = 1, r = 0, g = 0, b = 120 }
  --objs.brush1 = r:createbrush{ style ="solid", r = 200, g = 0, b = 200 }
  --objs.font1 = r:createfont{ font="fixedsys", height = 11, bold = 0 }

  regexp = createPcre("[0-9]+")
  values = {}
  regexp2 = cfg.regexp and createPcre(cfg.regexp) or nil
end

function statusbar.term(msg)
  cfg = nil
  values = {} 
  terminate(msg)
end
