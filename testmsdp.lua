-- testmsdp
-- Плагин Tortilla mud client

local testmsdp = {}
function testmsdp.name()
  return 'Тестовый плагин MSDP'
end

function testmsdp.description()
  return 'Плагин используется для тестирования протокола MSDP.'
end

function testmsdp.version()
  return '-'
end

local function log(s)
  print("rgb140,60,80", "[msdp]: "..s)
end

local w, r
local function render()
  local height = r:height()/r:fontHeight()
  local t = 'sample text'
  local c = 2
  r:textColor(props.paletteColor(c))
  local x = 0
  local y = 0
  r:print(x, y, t)
end

-- globals.lua
-- show all global variables

local seen={}

function dump(t,i)
	seen[t]=true
	local s={}
	local n=0
	for k in pairs(t) do
		n=n+1 s[n]=k
	end
	table.sort(s)
	for k,v in ipairs(s) do
		print(i,v)
		v=t[v]
		if type(v)=="table" and not seen[v] then
			dump(v,i.."\t")
		end
	end
end

local function indent(l)
  s = ""
  for i=0,l do
    s = s .. "  "
  end

  return s
end

function dump(o, level)
  if type(o) == 'table' then
    local s = indent(level) .. '{\n'
    for k,v in pairs(o) do
      if type(k) ~= 'number' then k = '"'..k..'"' end

      value = "<too deep>"
      if '"_G"' == k then
        value = "<skipped global>"
      elseif 3 > level then
        value = dump(v, 1 + level)
      end

      s = s .. indent(1 + level) .. '['..k..'] = ' .. value .. ',\n'
    end
    return s .. indent(level) .. '} '
  else
    return tostring(o)
  end
end

function exec(code)
  local call = load(code)
  if nil ~= call then
    log("Execution...")
    return call()
  end

  log(string.format("failed to load code '%s'", code))
end

local function message_handler(msg)
  log(msg)
end

local function subrange(t, first, last)
  local sub = {}
  for i=first,last do
    sub[#sub + 1] = t[i]
  end
  return sub
end

commands = {
  ["lua"] = function(code)
    local call = function () return exec(table.concat(code)) end
    return xpcall(call, message_handler)
  end,
  ["reload"] = function(code)
    local call = function () return dofile("plugins/testmsdp.lua") end
    return xpcall(call, message_handler)
  end,
  ["test"] = function(code) log('test') end
}

function testmsdp.syscmd(cmd)
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

function testmsdp.init()
  w = createWindow('MSDP автомаппер', 300, 300, true)
  if not w then
    terminate('Ошибка при создании окна.')
  end
  w:dock('right')
  r = w:setRender(render)
  r:setBackground( props.backgroundColor() )
  r:select(props.currentFont())
end

--[[
SENDABLE_VARIABLES
REPORTED_VARIABLES
REPORTABLE_VARIABLES
CONFIGURABLE_VARIABLES
COMMANDS
LISTS
]]

function testmsdp.msdpon()
  log("ON")
  msdp.list("LISTS")
  msdp.list("COMMANDS")
  msdp.list("REPORTABLE_VARIABLES")
  msdp.list("CONFIGURABLE_VARIABLES")

  msdp.report('ROOM')
  --msdp.report('ROOM_VNUM')
  --msdp.report('ROOM_NAME')
  msdp.report('MOVEMENT')
end

function testmsdp.msdpoff()
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

function testmsdp.msdp(t)
  if type(t) ~= 'table' then
    log('NOT TABLE!')
    return
  end
  dump_table('', t)
end

return testmsdp

-- * vim: ts=2 sw=2 et :
