-- msdpmapper
-- Плагин Tortilla mud client

-- Специальные константы на карте
local UNDEFINED = 0
local NOWHERE = -1
local AMBIGUOUS = -2

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
  me=colors.white,
  status=colors.yellow,
  unexplored=colors.white,
  ambiguous=colors.yellow
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

local avoid_tags = {dt=true}

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
  elseif AMBIGUOUS == cell then
    room_picture[3][2] = {"*", map_colors.ambiguous}
  else
    local path = {}
    if nil ~= msdpmapper.display_path then
      path = msdpmapper.display_path
    end
    local bracket_color = function()
      if nil ~= path[cell] then
        return map_colors.path
      end
      return map_colors.bracket
    end

    room_picture[2][2] = {"[", bracket_color()}
    room_picture[4][2] = {"]", bracket_color()}
    if nil == room then
      room_picture[3][2] = {"?", map_colors.unexplored}
    elseif msdpmapper.current_room == cell then
      room_picture[3][2] = {"@", map_colors.me}
    elseif nil ~= room["tags"]then
      local tag = nil
      for t, _ in pairs(room["tags"]) do
        if nil ~= room_tags[t] and nil == tag or room_tags[t].priority < tag.priority then
          tag = room_tags[t]
        end
      end

      if nil ~= tag then
        room_picture[3][2] = {tag.sign, tag.color}
      end
    end

    local color = function(d, c) local result = c
      if nil ~= path[cell] and path[cell][room.exits[d]] then
        result = map_colors.path
      end
      return result
    end
    local exit_color = function(d) return color(d, map_colors.exit) end

    if nil ~= room then
      if nil ~= room.exits["e"] then
        room_picture[5][2] = {"-", exit_color("e")}
      end
      if nil ~= room.exits["w"] then
        room_picture[1][2] = {"-", exit_color("w")}
      end
      if nil ~= room.exits["n"] then
        room_picture[3][1] = {"|", exit_color("n")}
      end
      if nil ~= room.exits["s"] then
        room_picture[3][3] = {"|", exit_color("s")}
      end
      if nil ~= room.exits["u"] then
        if nil ~= msdpmapper.rooms[room.exits["u"]] then
          room_picture[2][1] = {"^", exit_color("u")}
        else
          room_picture[2][1] = {"?", color("u", map_colors.unexplored)}
        end
      end
      if nil ~= room.exits["d"] then
        if nil ~= msdpmapper.rooms[room.exits["d"]] then
          room_picture[2][3] = {"v", exit_color("d")}
        else
          room_picture[2][3] = {"?", color("d", map_colors.unexplored)}
        end
      end
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
  draw_room = wide_rooms,
  walk_speed = 12,  -- commands per second
  max_walk_attempts = 24
}

local zones_filename = "zones.lua"
local rooms_filename = "rooms.lua"

-- Следующие две переменные нужны для обхода ошибки в функциях saveTable/loadTable: они некорректно созраняют ключи-числа
local vnum_prefix = "vnum_"
local zone_id_prefix = "zone_id_"

local default_map_width = 10
local default_map_height = 10

function msdpmapper.name()
  return 'Карта мира (msdp)'
end

function msdpmapper.description()
  local s = { 'Плагин русует карту с помощью протокола MSDP.',
  'Требуется поддержка MSDP с информацией о местоположении игрока на сервере.' }
  return table.concat(s, '\r\n')
end

function msdpmapper.version()
  return '1.01'
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

  local auto_path = "off"
  if nil ~= msdpmapper.auto_path then
    auto_path = string.format("on (to room %s)", msdpmapper.auto_path)
  end

  local speedwalk = "off"
  if msdpmapper.speedwalk_on then
    speedwalk = "on"
  end

  local current_room = "<undefined>"
  if nil ~= msdpmapper.current_room then
    current_room = msdpmapper.current_room
  end

  local status = string.format("Current room: %s; Auto path: %s; Speed walk: %s", current_room, auto_path, speedwalk)
  renderer:textColor(map_colors.status)
  renderer:print(0, 0, status)
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

  -- zones and rooms can be false, if file is missing
  if zones and rooms then
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

--[[
Комната на карте является неоднозначной, если хотя бы один из её выходов ведёт в комнату, которая не соответствует уже находящейся в ячейке, соответствеющей направлению,
либо если одна из соседних комнат имеет выход в данную ячейку на карте, но выход ведёт в комнату с другим VNUM.

Пример 1:

[?] -[3]
 |    |
[2]--[1]

Допустим, выход на запад из [3] ведёт в [5], но мы ещё не успели нарисовать эту комнату. Зато нам уже нужно поставить комнату [4], в которую ведёт выход на север из комнаты [2]. Если бы мы нарисовали комнату [4], то получили бы следующую карту:

[4] -[3]
 |    |
[2]--[1]

Которая является неоднозначной, т. к. выглядит так, будто из [3] есть выход в [4], тогда как на самом деле - это выход в [5].

Пример 2:

[?]  [3]
 |    |
[2]--[1]

Допустим, на нужно поставить в ячейку [?] комнату с VNUM 4, у которой есть выход на восток в комнату 5. Если нарисовать такую комнату, то мы получим следующую карту:

[4]- [3]
 |    |
[2]--[1]

Которая будет неоднозначной, т. к. выглядит так, будто из [4] есть выход в [3], тогда как он ведёт совсем в другую комнату.

Функция filter_ambiguous проверяет такие неоднозначности и вместо неоднозначной комнаты ставит специальную пометку AMBIGUOUS
]]
local function filter_ambiguous(map, x, y, vnum, width, height)
  -- проверка западного направления из комнаты vnum
  if 1 < x then
    -- Проверка на первый тип неоднозначности
    local cell = map[x - 1][y]
    if 0 < tonumber(cell) then
      local room = msdpmapper.rooms[cell]
      if nil ~= room then
        local east_to = room.exits["e"]
        if nil ~= east_to and east_to ~= vnum then  -- комната на западе имеет выход на восток и он ведёт не в ту комнату, которую мы собираемся нарисовать: неоднозначность
          return AMBIGUOUS
        end
      end

      -- Проверка на второй тип неоднозначности
      local room = msdpmapper.rooms[vnum]
      if nil ~= room and nil ~= room.exits["w"] then
        if cell ~= room.exits["w"] then  -- выход на запад из комнаты, которую мы собираемся нарисовать, ведёт не в ту комнату, которая там уже нарисована: неоднозначность
          return AMBIGUOUS
        end
      end
    end
  end

  -- проверка восточного направления из комнаты vnum
  if x < width - 1 then
    local cell = map[x + 1][y]
    if 0 < tonumber(cell) then
      -- Проверка на первый тип неоднозначности
      local room = msdpmapper.rooms[cell]
      if nil ~= room then
        local west_to = room.exits["w"]
        if nil ~= west_to and west_to ~= vnum then  -- комната на востоке имеет выход на запад и он ведёт не в ту комнату, которую мы собираемся нарисовать: неоднозначность
          return AMBIGUOUS
        end
      end

      -- Проверка на второй тип неоднозначности
      local room = msdpmapper.rooms[vnum]
      if nil ~= room and nil ~= room.exits["e"] then
        if cell ~= room.exits["e"] then  -- выход на восток из комнаты, которую мы собираемся нарисовать, ведёт не в ту комнату, которая там уже нарисована: неоднозначность
          return AMBIGUOUS
        end
      end
    end
  end

  -- проверка северного направления из комнаты vnum
  if 1 < y then
    -- Проверка на первый тип неоднозначности
    local cell = map[x][y - 1]
    if 0 < tonumber(cell) then
      local room = msdpmapper.rooms[cell]
      if nil ~= room then
        local south_to = room.exits["s"]
        if nil ~= south_to and south_to ~= vnum then  -- комната на севере имеет выход на юг и он ведёт не в ту комнату, которую мы собираемся нарисовать: неоднозначность
          return AMBIGUOUS
        end
      end

      -- Проверка на второй тип неоднозначности
      local room = msdpmapper.rooms[vnum]
      if nil ~= room and nil ~= room.exits["n"] then
        if cell ~= room.exits["n"] then  -- выход на север из комнаты, которую мы собираемся нарисовать, ведёт не в ту комнату, которая там уже нарисована: неоднозначность
          return AMBIGUOUS
        end
      end
    end
  end

  -- проверка южного направления из комнаты vnum
  if y < height - 1 then
    -- Проверка на первый тип неоднозначности
    local cell = map[x][y + 1]
    if 0 < tonumber(cell) then
      local room = msdpmapper.rooms[cell]
      if nil ~= room then
        local north_to = room.exits["n"]
        if nil ~= north_to and north_to ~= vnum then  -- комната на юге имеет выход на север и он ведёт не в ту комнату, которую мы собираемся нарисовать: неоднозначность
          return AMBIGUOUS
        end
      end

      -- Проверка на второй тип неоднозначности
      local room = msdpmapper.rooms[vnum]
      if nil ~= room and nil ~= room.exits["s"] then
        if cell ~= room.exits["s"] then  -- выход на юг из комнаты, которую мы собираемся нарисовать, ведёт не в ту комнату, которая там уже нарисована: неоднозначность
          return AMBIGUOUS
        end
      end
    end
  end

  return vnum
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
      local cell = filter_ambiguous(map, x, y, next_room.vnum, width, height)
      map[x][y] = cell

      local room = msdpmapper.rooms[next_room.vnum]
      if nil ~= room and AMBIGUOUS ~= cell then -- Комната, в которую ведёт выход может ещё не присутствовать в данных маппера, плюс не нужно рисовать маршрут из неоднозначных комнат
        for d, v in pairs(room.exits) do
          -- Наша карта - плоская. Поэтому нет необходимости помещать в очередь комнаты сверху и снизу
          if 'e' == d then
            if x < width - 1 then
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
            if y < height - 1 then
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
  if 1 > #arguments then
    log("Not enough arguments.")
    log("Usage 1: #tag <vnum> <tag>")
    log("Usage 2 (to tag current room): #tag <tag>")
    log("Allowed tags: " .. keys_concat(room_tags))
    return
  end

  local vnum = msdpmapper.current_room
  local tag = nil;
  if 2 == #arguments then
    vnum = arguments[1]
    tag = table.concat(subrange(arguments, 2, #arguments), " ")
  else
    tag = arguments[1]
  end

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
  if 1 > #arguments then
    log("Not enough arguments.")
    log("Usage 1: #untag <vnum> <tag>")
    log("Usage 2 (to untag current room): #untag <tag>")
    return
  end

  local vnum = msdpmapper.current_room
  local tag = nil;
  if 2 == #arguments then
    vnum = arguments[1]
    tag = table.concat(subrange(arguments, 2, #arguments), " ")
  else
    tag = arguments[1]
  end

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
  if 0 == #msdpmapper.rooms[vnum]["tags"] then
    msdpmapper.rooms[vnum]["tags"] = nil
  end
  log(string.format("Tag '%s' successfully remove from room %d.", tag, vnum))
end

local function avoid(vnum)
  local room = msdpmapper.rooms[vnum]
  if nil == room or nil == room["tags"] then
    return false
  end

  for tag, _ in pairs(room["tags"]) do
    if nil ~= avoid_tags[tag] then
      return true
    end
  end

  return false
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
        if nil == reverse_path[v] and not avoid(v) then
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
    log(string.format("Starting room with VNUM %d not found.", vnum_from))
    return false
  end
  if nil == msdpmapper.rooms[vnum_to] then
    log(string.format("Target room with VNUM %d not found.", vnum_to))
    return false
  end

  local path = msdpmapper.get_path(vnum_from, vnum_to)
  if nil == path then
    log("Path not found")
    return false
  end

  msdpmapper.path = path
  msdpmapper.display_path = join_path(path)
  msdpmapper.renderer:update()
  return true
end

function msdpmapper.print_path(arguments)
  local vnum_from = arguments[1]
  local vnum_to = arguments[2]
  if nil == msdpmapper.rooms[vnum_from] then
    log(string.format("Starting room with VNUM %d not found.", vnum_from))
    return
  end
  if nil == msdpmapper.rooms[vnum_to] then
    log(string.format("Target room with VNUM %d not found.", vnum_to))
    return
  end

  local path = msdpmapper.get_path(vnum_from, vnum_to)
  if nil == path then
    log("Path not found")
    return false
  end

  local flat_path = flatten_path(path, vnum_from)
  log(table.concat(flat_path, ", "))
  return true
end

function msdpmapper.update_auto_path()
  if nil ~= msdpmapper.current_room and nil ~= msdpmapper.auto_path then
    return msdpmapper.set_path({msdpmapper.current_room, msdpmapper.auto_path})
  end

  return false
end

function msdpmapper.set_auto_path(arguments)
  local destination = table.concat(arguments, "")
  if nil == msdpmapper.rooms[destination] then
    log(string.format("Room with VNUM %d does not exist. Setting auto path cancelled.", destination))

    return false
  end

  msdpmapper.auto_path = destination
  return msdpmapper.update_auto_path()
end

local expected_command = nil
function msdpmapper.walk()
  local from = msdpmapper.current_room
  local room = msdpmapper.rooms[from]
  local to = msdpmapper.path[from]
  if nil == from then
    log("Speed walk failed because current room is undefined.")
    return
  elseif nil == room then
    log("Speed walk failed because current room not found in the world (most likely that is an internal error).")
    return
  elseif nil == to then
    log("Speed walk failed because current room not found in the path (most likely that is an internal error).")
    return
  end

  local counter = msdpmapper.walk_speed
  local commands = {}
  repeat
    for d, r in pairs(room.exits) do
      if r == to then
        table.insert(commands, d)

        room = msdpmapper.rooms[to]
        if nil == msdpmapper.path[to] or nil == room or msdpmapper.auto_path == to then
          break
        end

        msdpmapper.expected_room = to
        to = msdpmapper.path[to]
      end
    end

    if 0 < counter then
      counter = counter - 1
    end
  until 0 == counter

  if 0 == #commands then
    log(string.format("Couldn't find exit to room %d (most likely that is an internal error)", to))
    return
  end

  for _, command in pairs(commands) do
    expected_command = command
    runCommand(command)
  end
end

function msdpmapper.speedwalk(arguments)
  if 0 == #arguments then
    if nil == msdpmapper.auto_path then
      log("Speedwalk available only with auto path.")
      return
    elseif nil == msdpmapper.path then
      log("No route to destination.")
      return
    end
  else
    if not msdpmapper.set_auto_path(arguments) then
      log("Failed to set auto path to " .. table.concat(arguments, ""))
      return
    end
  end

  if msdpmapper.current_room ~= msdpmapper.auto_path then
    msdpmapper.speedwalk_on = true
    msdpmapper.walk_wait_counter = 0
    msdpmapper.walk()
  else
    log("You are already there.")
  end
end

local move_commands = {
  ["с"]="n",
  ["ю"]="s",
  ["в"]="e",
  ["з"]="w",
  ["вв"]="u",
  ["вн"]="d"
}
local check_directions={
  n=true,
  s=true,
  e=true,
  w=true,
  u=true,
  d=true
}

function msdpmapper.get_destination(command)
  if nil == msdpmapper.current_room then
    return nil
  end

  local room = msdpmapper.rooms[msdpmapper.current_room]
  if nil == room or nil == room["exits"] then
    return nil
  end

  local destination = nil
  if nil ~= check_directions[command[1]] then
    destination = room["exits"][command[1]]
  elseif nil ~= move_commands[command[1]] and check_directions[move_commands[command[1]]] then
    destination = room["exits"][move_commands[command[1]]]
  end

  if nil == destination then
    return nil
  end

  return destination
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
  ["tag"] = msdpmapper.tag_room,
  ["untag"] = msdpmapper.untag_room,
  ["print_path"] = msdpmapper.print_path,
  ["show_path"] = msdpmapper.set_path,
  ["auto_path"] = msdpmapper.set_auto_path,
  ["speedwalk"] = msdpmapper.speedwalk
}

function msdpmapper.syscmd(cmd)
  if nil ~= cmd then
    if "msdpmapper" == cmd[1] then
      arguments = subrange(cmd, 3, #cmd)
      for k, v in pairs(commands) do
        if k == cmd[2] then
          local call = function() v(arguments) end
          return xpcall(call, message_handler)
        end
      end
    end
  end

  return cmd
end

function msdpmapper.gamecmd(cmd)
  -- log(table.concat(cmd, " "))
  if msdpmapper.speedwalk_on and cmd[1] ~= expected_command then
    log("Speedwalk interrupted by user command.")
    msdpmapper.speedwalk_on = false
    msdpmapper.renderer:update()
  end

  local destination = msdpmapper.get_destination(cmd)
  if nil ~= destination then
    if avoid(destination) then
      log(string.format("MSDP mapper blocked movement to dangerous room %s (%s).", destination, msdpmapper.rooms[destination]["name"]))
      return false
    end
  end

  return cmd
end

function msdpmapper.init()
  if createWindowDpi then
    msdpmapper.window = createWindowDpi('MSDP автомаппер', 300, 300, true)
  else
    msdpmapper.window = createWindow('MSDP автомаппер', 300, 300, true)
  end
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

function msdpmapper.tick()
  if msdpmapper.speedwalk_on then
    if msdpmapper.current_room == msdpmapper.expected_room then
      msdpmapper.walk_wait_counter = 0
      msdpmapper.walk()
    elseif nil ~= msdpmapper.walk_wait_counter and msdpmapper.max_walk_attempts > msdpmapper.walk_wait_counter then
      msdpmapper.walk_wait_counter = 1 + msdpmapper.walk_wait_counter
      log(string.format("Speed walk expected us to be in room '%s' but we are in '%s'. Check %d of %d.",
          msdpmapper.expected_room,
          msdpmapper.current_room,
          msdpmapper.walk_wait_counter,
          msdpmapper.max_walk_attempts))
    else
      msdpmapper.speedwalk_on = false
      log(string.format("Speedwalk turned off because it stuck (wait counter: %d).", msdpmapper.walk_wait_counter))
    end
  end
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

  if vnum == msdpmapper.auto_path then
    log(string.format("You have reached your destination (%s: %s)",
        vnum, msdpmapper.rooms[vnum]["name"]))
    msdpmapper.speedwalk_on = false
    msdpmapper.auto_path = nil
    msdpmapper.display_path = nil
  end

  msdpmapper.update_auto_path()
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
