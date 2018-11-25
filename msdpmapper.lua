-- msdpmapper
-- Плагин Tortilla mud client

local msdpmapper = {
  ["rooms"] = {},
  ["zones"] = {},
  current_room = nil,
  current_map = nil,
  renderer = nil,
  window = nil
}

local zones_filename = "msdpmapper.zones.lua"
local rooms_filename = "msdpmapper.rooms.lua"

-- workaround to make loadTable/saveTable to work
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

  local c = 2
  renderer:textColor(props.paletteColor(c))
  map = transpose(msdpmapper.current_map)
  for y, row in pairs(map) do
    local line = ""
    for x, cell in pairs(row) do
      if UNDEFINED == cell then
        line = line .. "#"
      elseif NOWHERE == cell then
        line = line .. "?"
      elseif msdpmapper.current_room == cell then
        line = line .. "@"
      else
        line = line .. " "
      end
    end
    renderer:print(0, (y - 1)*renderer:fontHeight(), line)
  end
end

function msdpmapper.render()
  xpcall(unprotected_render, message_handler)
end

function exec(code)
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
    width = default_map_width
  end

  if nil == height then
    height = default_map_height
  end

  local map = {}
  for x=1,width do
    map[x] = {}     -- create a new row
    for y=1,height do
      map[x][y] = UNDEFINED
    end
  end

  local cx = width / 2
  local cy = height / 2

  queue = {}
  if nil == msdpmapper.current_room then
    map[cx][cy] = NOWHERE
  else
    table.insert(queue, {vnum=msdpmapper.current_room, x=cx, y=cy})
  end

  local seen = {}
  while 0 ~= #queue do
    local next_room = table.remove(queue, 1)  -- TODO: removing is not efficient
    local x = next_room.x
    local y = next_room.y
    if nil == seen[next_room.vnum] and UNDEFINED == map[x][y] then
      -- process unseen room
      map[x][y] = next_room.vnum

      local room = msdpmapper.rooms[next_room.vnum]
      if nil ~= room then -- room where some exit leads to, may be not explored yet
        for d, v in pairs(room.exits) do
          -- don't enqueue rooms in up and down directions
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

commands = {
  ["lua"] = function(code)
    local call = function () return exec(table.concat(code, " ")) end
    return xpcall(call, message_handler)
  end,
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
    return result
  end,
  ["loadmaps"] = msdpmapper.load,
  ["savemaps"] = msdpmapper.save,
  ["draw"] = protected_draw_map
}

function msdpmapper.syscmd(cmd)
  if nil ~= cmd then
    arguments = subrange(cmd, 2, #cmd)
    for k, v in pairs(commands) do
      if k == cmd[1] then
        return v(arguments)
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
