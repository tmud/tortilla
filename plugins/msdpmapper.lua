-- msdpmapper
-- Плагин Tortilla mud client

local colors = {
  white=0xffffff,
  lime=0x00ff00,
  green=0x008800,
  red=0x0000ff,
  dark_blue=0x880000,
  blue=0xff0000,
  yellow=0x00ffff,
  grey=0x888888,
  black=0x000000
}

local map_colors = {
  unknown_position=colors.grey,
  exit=colors.green,
  bracket=colors.grey,
  path=0xff8888,
  me=colors.white
}

local room_tags = {
  dt={
    color=colors.red,
    sign="Ж",
    priority=1  -- Если ячейка имеет несколько тегов, то приоритет будет определять обозначение клетки (чем меньше значение - тем выше приоритет)
  },
  quest={
    color=colors.yellow,
    sign="Q",
    priority=2
  },
  dealer={
    color=colors.white,
    sign="$",
    priority=4
  },
  bank={
    color=colors.white,
    sign="B",
    priority=5
  },
  rent={
    color=colors.white,
    sign="R",
    priority=3
  },
  teacher={
    color=colors.white,
    sign="T",
    priority=6
  },
  safe={
    color=colors.grey,
    sign="~",
    priority=7
  }
}

local function draw_tight_room(x, y, cell, renderer)
  local line = " "
  if UNDEFINED == cell then
    line = "#"
  elseif NOWHERE == cell then
    line = "?"
  elseif msdpmapper.current_room == cell then
    line = "@"
  end
  renderer:print((x - 1)*renderer:textWidth(' '), (y - 1)*renderer:fontHeight(), line)
end

local tight_rooms = {
  width = 1,
  height = 1,
  draw = draw_tight_room
}

local wide_rooms = {
  width = 5,
  height = 3
}

wide_rooms.draw = function(x, y, cell, renderer)
  local room = msdpmapper.rooms[cell]
  if nil == room then
    return
  end

  local room_width = wide_rooms.width
  local room_height = wide_rooms.height
  local room_picture = {}
  for px=1,room_width do
    room_picture[px] = {}
    for py=1,room_height do
      room_picture[px][py] = {" ", colors.white}
    end
  end

  local char_width = renderer:textWidth(' ')
  local char_height = renderer:fontHeight()

  if UNDEFINED == cell then
  elseif NOWHERE == cell then
    room_picture[3][2] = {"?", map_colors.unknown_position}
  else
    local path = {}
    if nil ~= msdpmapper.path then
      path = msdpmapper.path
    end
    local bracket_color = function()
      if nil ~= path[cell] then
        return map_colors.path
      end
      return map_colors.bracket
    end

    room_picture[2][2] = {"[", bracket_color()}
    room_picture[4][2] = {"]", bracket_color()}
    if msdpmapper.current_room == cell then
      room_picture[3][2] = {"@", map_colors.me}
    elseif nil ~= room["tags"] then
      local tag = nil
      for t, _ in pairs(room["tags"]) do
        if nil ~= room_tags[t] and nil == tag or room_tags[t].priority < tag.priority then
          tag = room_tags[t]
        end
      end

      room_picture[3][2] = {tag.sign, tag.color}
    end

    local color = function(d) local result = map_colors.exit
      if nil ~= path[cell] and path[cell][room.exits[d]] then
        result = map_colors.path
      end
      return result
    end

    if nil ~= room.exits["e"] then
      room_picture[5][2] = {"-", color("e")}
    end
    if nil ~= room.exits["w"] then
      room_picture[1][2] = {"-", color("w")}
    end
    if nil ~= room.exits["n"] then
      room_picture[3][1] = {"|", color("n")}
    end
    if nil ~= room.exits["s"] then
      room_picture[3][3] = {"|", color("s")}
    end
    if nil ~= room.exits["u"] then
      room_picture[1][1] = {"^", color("u")}
    end
    if nil ~= room.exits["d"] then
      room_picture[4][3] = {"v", color("d")}
    end
  end

  for py=1,room_height do
    for px=1,room_width do
      renderer:textColor(room_picture[px][py][2])
      renderer:print((x - 1)*char_width*room_width + px*char_width, (y - 1)*char_height*room_height + py*char_height, room_picture[px][py][1])
    end
  end
end

local msdpmapper = {
  ["rooms"] = {},
  ["zones"] = {},
  current_room = nil,
  current_map = nil,
  renderer = nil,
  window = nil,
  draw_room = wide_rooms
}

local zones_filename = "msdpmapper.zones.lua"
local rooms_filename = "msdpmapper.rooms.lua"

-- Следующие две переменные нужны для обхода ошибки в функциях saveTable/loadTable: они некорректно созраняют ключи-числа
local vnum_prefix = "vnum_"
local zone_id_prefix = "zone_id_"

local default_map_width = 10
local default_map_height = 10

local UNDEFINED = 0
local NOWHERE = -1

function msdpmapper.name()
  return 'Тестовый плагин MSDP'
end

function msdpmapper.description()
  return 'Плагин используется для тестирования протокола MSDP.'
end

function msdpmapper.version()
  return '-'
end

local function log(s)
  print("rgb140,60,80", "[msdp]: "..s)
end

local function message_handler(msg)
  log(msg)
end

local function dump_helper(o, level, prefix)
  local indent = "  "
  if type(o) == 'table' then
    log(prefix .. '{')
    for k,v in pairs(o) do
      if type(k) ~= 'number' then k = '"'..k..'"' end

      prefix = string.rep(indent, 1 + level) .. '['..k..'] = '
      if '"_G"' == k then
        log(prefix .. "<skipped global>")
      elseif 4 > level then
        dump_helper(v, 1 + level, prefix)
      else
        log(prefix .. "<too deep>")
      end
    end
    log(string.rep(indent, level) .. '} ')
  elseif type(o) == "function" then
    log(prefix .. "<function>")
  elseif type(o) ~= "string" then
    log(prefix .. string.format("%s", o))
  else
    log(prefix .. string.format("\"%s\"", o))
  end
end

function dump(o)
  dump_helper(o, 0, "")
end

local function transpose(m)
  local rotated = {}
  for c, m_1_c in ipairs(m[1]) do
    local col = {m_1_c}
    for r = 2, #m do
      col[r] = m[r][c]
    end
    table.insert(rotated, col)
  end
  return rotated
end

local function unprotected_render()
  local renderer = msdpmapper.renderer
  if nil == msdpmapper.current_map then
    return
  end

  map = transpose(msdpmapper.current_map)
  for y, row in pairs(map) do
    for x, cell in pairs(row) do
      msdpmapper.draw_room.draw(x, y, cell, renderer)
    end
  end
end

function msdpmapper.render()
  xpcall(unprotected_render, message_handler)
end

function exec(arguments)
  local code = table.concat(arguments, " ")
  local call = load(code)
  if nil ~= call then
    log("Execution...")
    return call()
  end

  log(string.format("failed to load code '%s'", code))
end

local function subrange(t, first, last)
  local sub = {}
  for i=first,last do
    sub[#sub + 1] = t[i]
  end
  return sub
end

function msdpmapper.save()
  local zones = {}
  for z, d in pairs(msdpmapper.zones) do
    zones[zone_id_prefix .. z] = d
  end

  local rooms = {}
  for vnum, r in pairs(msdpmapper.rooms) do
    rooms[vnum_prefix .. vnum] = r
  end

  local zones_save_result = saveTable({zones}, zones_filename)
  local rooms_save_result = saveTable({rooms}, rooms_filename)

  if nil == zones_save_result then
    log("Не удалось сохранить список зон.")
  end

  if nil == rooms_save_result then
    log("Не удалось сохранить список комнат.")
  end

  if zones_save_result and rooms_save_result then
    log("Данные мира сохранены успешно.")
  end
end

function msdpmapper.load()
  local zones = loadTable(zones_filename)
  local rooms = loadTable(rooms_filename)

  if nil == zones then 
    log("Файл с зонами содержит ошибку. Загрузка отменена.")
  end

  if nil == rooms then
    log("Файл с комнатами содержит ошибку. Загрузка отменена.")
  end

  if nil ~= zones and nil ~= rooms then
    msdpmapper.rooms = {}
    for r, v in pairs(rooms[1]) do
      msdpmapper.rooms[string.gsub(r, vnum_prefix, "")] = v
    end

    msdpmapper.zones = {}
    for z, d in pairs(zones[1]) do
      msdpmapper.zones[string.gsub(z, zone_id_prefix, "")] = d
    end

    log("Данные мира успешно загружены.")
  end
end

function msdpmapper.draw_map(width, height)
  if nil == width then
    width = math.floor(msdpmapper.renderer:width()/(msdpmapper.renderer:textWidth(' ')*msdpmapper.draw_room.width))
  end

  if nil == height then
    height = math.floor(msdpmapper.renderer:height()/(msdpmapper.renderer:fontHeight()*msdpmapper.draw_room.height))
  end

  local map = {}
  for x=1,width do
    map[x] = {}
    for y=1,height do
      map[x][y] = UNDEFINED
    end
  end

  local cx = math.floor(width / 2)
  local cy = math.floor(height / 2)

  queue = {}
  if nil == msdpmapper.current_room then
    map[cx][cy] = NOWHERE
  else
    table.insert(queue, {vnum=msdpmapper.current_room, x=cx, y=cy})
  end

  local seen = {}
  while 0 ~= #queue do
    local next_room = table.remove(queue, 1)  -- TODO: удаление не эффективно, т. к. удаление из начала массива ведёт к сдвигу всего остатка массива в начало
    local x = next_room.x
    local y = next_room.y
    if nil == seen[next_room.vnum] and UNDEFINED == map[x][y] then
      -- Обработка ещё не обработанной комнаты
      map[x][y] = next_room.vnum

      local room = msdpmapper.rooms[next_room.vnum]
      if nil ~= room then -- Комната, в которую ведёт выход может ещё не присутствовать в данных маппера
        for d, v in pairs(room.exits) do
          -- Наша карта - плоская. Поэтому нет необходимости помещать в очередь комнаты сверху и снизу
          if 'e' == d then
            if x < width then
              table.insert(queue, {vnum=v, x=1 + x, y=y})
            end
          elseif 'w' == d then
            if 1 < x then
              table.insert(queue, {vnum=v, x=x - 1, y=y})
            end
          elseif 'n' == d then
            if 1 < y then
              table.insert(queue, {vnum=v, x=x, y=y - 1})
            end
          elseif 's' == d then
            if y < height then
              table.insert(queue, {vnum=v, x=x, y=1 + y})
            end
          end
        end
      end
    end

    seen[next_room.vnum] = true
  end

  msdpmapper.current_map = map
end

local function protected_draw_map()
  result = xpcall(msdpmapper.draw_map, message_handler)
  if not result then
    log("Failed to draw map.")
  end
  return result
end

local function keys_concat(t)
  local keys = {}
  local n = 1
  for key, _ in pairs(t) do
    keys[n] = key
    n = n + 1
  end

  return table.concat(keys, ", ")
end

function msdpmapper.tag_room(arguments)
  if 2 > #arguments then
    log("Not enough arguments.")
    log("Usage: #tag_room <vnum> <tag>")
    log("Allowed tags: " .. keys_concat(room_tags))
    return
  end

  local vnum = arguments[1]
  local tag = table.concat(subrange(arguments, 2, #arguments), " ")

  local error = false
  if nil == msdpmapper.rooms[vnum] then
    log(string.format("Room with vnum %d not found.", vnum))
    error = true
  end

  if nil == room_tags[tag] then
    local allowed = keys_concat(room_tags)
    log(string.format("Tag '%s' is not allowed. Allowed tags: %s.", tag, allowed))
    error = true
  end

  if error then
    return
  end

  if nil == msdpmapper.rooms[vnum]["tags"] then
    msdpmapper.rooms[vnum]["tags"] = {}
  end
  msdpmapper.rooms[vnum]["tags"][tag] = 1

  log(string.format("Tag '%s' successfully added to room %d.", tag, vnum))
end

function msdpmapper.untag_room(arguments)
  if 2 > #arguments then
    log("Not enough arguments.")
    log("Usage: #untag_room <vnum> <tag>")
    return
  end

  local vnum = arguments[1]
  local tag = table.concat(subrange(arguments, 2, #arguments), " ")

  local error = false
  if nil == msdpmapper.rooms[vnum] then
    log(string.format("Room with vnum %d not found.", vnum))
    error = true
  end

  if nil == msdpmapper.rooms[vnum]["tags"] then
    msdpmapper.rooms[vnum]["tags"] = {}
  end
  if nil == msdpmapper.rooms[vnum]["tags"][tag] then
    log(string.format("Room %s doesn't have tag '%s'.", vnum, tag))
    error = true
  end

  if error then
    return
  end

  msdpmapper.rooms[vnum]["tags"][tag] = nil
  log(string.format("Tag '%s' successfully remove from room %d.", tag, vnum))
end

function msdpmapper.get_path(vnum_from, vnum_to)
  if vnum_from == vnum_to then
    return {} -- empty path means that we already in the target room
  end
  local queue = {vnum_from}
  local current = 1

  local step = 1
  local reverse_path = {[vnum_from]=NOWHERE}
  local next_queue = {}
  while 0 ~= #queue do
    next_vnum = queue[current]
    --log("Processing room " .. next_vnum)
    if vnum_to == next_vnum then
      break
    end

    room = msdpmapper.rooms[next_vnum]
    if nil ~= room then
      for d, v in pairs(room.exits) do
        if nil == reverse_path[v] then
          reverse_path[v] = next_vnum
          table.insert(next_queue, v)
          --log("Enqueue room " .. v .. " at step " .. step)
        end
      end
    end

    current = 1 + current
    if current > #queue then
      --log("Switching to step " .. (1 + step))
      current = 1
      queue = next_queue
      next_queue = {}
      step = 1 + step
    end
  end

  if 0 == #queue then
    return nil -- path not found
  end

  local path = {}
  local current = vnum_to
  while current ~= vnum_from do
    path[reverse_path[current]] = current
    current = reverse_path[current]
  end
  return path
end

local function flatten_path(path, from)
  local passed = {}
  local result = {}
  while nil ~= from do
    table.insert(result, path[from])
    passed[from] = true
    from = path[from]
  end

  return result
end

local function join_path(path)
  local result = {}
  for f, t in pairs(path) do
    if nil == result[f] then
      result[f] = {}
    end
    result[f][t] = true

    if nil == result[t] then
      result[t] = {}
    end
    result[t][f] = true
  end

  return result
end

function msdpmapper.set_path(arguments)
  local vnum_from = arguments[1]
  local vnum_to = arguments[2]
  if nil == msdpmapper.rooms[vnum_from] then
    log(string.format("Start room with VNUM %d not found.", vnum_from))
    return
  end
  if nil == msdpmapper.rooms[vnum_to] then
    log(string.format("Target room with VNUM %d not found.", vnum_to))
    return
  end

  local path = msdpmapper.get_path(vnum_from, vnum_to)
  msdpmapper.path = join_path(path)
  msdpmapper.renderer:update()
end

function msdpmapper.print_path(arguments)
  local vnum_from = arguments[1]
  local vnum_to = arguments[2]
  if nil == msdpmapper.rooms[vnum_from] then
    log(string.format("Start room with VNUM %d not found.", vnum_from))
    return
  end
  if nil == msdpmapper.rooms[vnum_to] then
    log(string.format("Target room with VNUM %d not found.", vnum_to))
    return
  end

  local path = msdpmapper.get_path(vnum_from, vnum_to)
  if nil == path then
    log("Path not found")
  else
    local flat_path = flatten_path(path, vnum_from)
    log(table.concat(flat_path, ", "))
  end
end

local commands = {
  ["lua"] = exec,
  ["reload"] = function(code)
    local call = function ()
      local window = msdpmapper.window
      local current_room = msdpmapper.current_room
      msdpmapper = dofile("plugins/msdpmapper.lua")
      _G["msdpmapper"] = msdpmapper
      msdpmapper.current_room = current_room
      msdpmapper.window = window
      msdpmapper.renderer = window:setRender(msdpmapper.render)
      msdpmapper.renderer:setBackground(props.backgroundColor())
      msdpmapper.renderer:select(props.currentFont())
      msdpmapper.load()
      protected_draw_map()
      msdpmapper.renderer:update()
    end

    result = xpcall(call, message_handler)
    if result then
      log("MSDP Mapper plugin has been reloaded.")
    end
  end,
  ["loadmaps"] = msdpmapper.load,
  ["savemaps"] = msdpmapper.save,
  ["tag_room"] = msdpmapper.tag_room,
  ["untag_room"] = msdpmapper.untag_room,
  ["print_path"] = msdpmapper.print_path,
  ["show_path"] = msdpmapper.set_path
}

function msdpmapper.syscmd(cmd)
  if nil ~= cmd then
    arguments = subrange(cmd, 2, #cmd)
    for k, v in pairs(commands) do
      if k == cmd[1] then
        local call = function() v(arguments) end
        return xpcall(call, message_handler)
      end
    end
  end

  return cmd
end

function msdpmapper.init()
  msdpmapper.window = createWindow('MSDP автомаппер', 300, 300, true)
  if not msdpmapper.window then
    terminate('Ошибка при создании окна.')
  end
  msdpmapper.window:dock('right')
  msdpmapper.renderer = msdpmapper.window:setRender(msdpmapper.render)
  msdpmapper.renderer:setBackground(props.backgroundColor())
  msdpmapper.renderer:select(props.currentFont())
  msdpmapper.load()
end

--[[
SENDABLE_VARIABLES
REPORTED_VARIABLES
REPORTABLE_VARIABLES
CONFIGURABLE_VARIABLES
COMMANDS
LISTS
]]

function msdpmapper.msdpon()
  log("ON")
  msdp.list("LISTS")
  msdp.list("COMMANDS")
  msdp.list("REPORTABLE_VARIABLES")
  msdp.list("CONFIGURABLE_VARIABLES")

  msdp.report('ROOM')
  --msdp.report('ROOM_VNUM')
  --msdp.report('ROOM_NAME')
  --msdp.report('MOVEMENT')
end

function msdpmapper.msdpoff()
  msdpmapper.save()
  log("OFF")
  msdp.unreport("ROOM")
  msdp.unreport("MOVEMENT")
end

local function dump_table(level,t)
  for k,v in pairs(t) do
    local s = level..'['..k..']='
    if (type(v) == 'string') then
      s = s..tostring(v)
    end
    log(s)
    if type(v) == 'table' then
      dump_table(level..'  ', v)
    end
  end
end

function msdpmapper.add_room(room)
  local zone_id = room["ZONE"]
  if nil == zone_id then
    log("MSDP variable ROOM without ZONE ID")
    return false
  end

  local vnum = room["VNUM"]
  if nil == vnum then
    log("MSDP variable ROOM without room VNUM")
    return false
  end

  if nil == msdpmapper.zones[zone_id] then
    msdpmapper.zones[zone_id] = {}
  end

  msdpmapper.zones[zone_id]["name"] = room["AREA"]

  if nil == msdpmapper.rooms[vnum] then
    msdpmapper.rooms[vnum] = {}
  end
  msdpmapper.rooms[vnum]["name"] = room["NAME"]
  msdpmapper.rooms[vnum]["terrain"] = room["TERRAIN"]
  msdpmapper.rooms[vnum]["exits"] = room["EXITS"]

  return true
end

function msdpmapper.set_current_room(vnum)
  if nil == vnum or nil == msdpmapper.rooms[vnum] then
    return
  end

  msdpmapper.current_room = vnum
end

function msdpmapper.msdp(t)
  if type(t) ~= 'table' then
    log('NOT TABLE!')
    return
  end

  if nil ~= t["ROOM"] then
    msdpmapper.add_room(t["ROOM"])
    msdpmapper.set_current_room(t["ROOM"]["VNUM"])
    msdpmapper.draw_map()
    msdpmapper.renderer:update()
  end

--  dump_table(0, t)
end

return msdpmapper

-- * vim: ts=2 sw=2 et :
