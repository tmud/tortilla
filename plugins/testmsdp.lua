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
-- msdp.list("LISTS")
-- msdp.list("COMMANDS")
-- msdp.list("REPORTABLE_VARIABLES")
-- msdp.list("CONFIGURABLE_VARIABLES")
  msdp.report('ROOM')
  msdp.report('MOVEMENT')
--   msdp.report{'ROOM', 'MOVEMENT'}
end

function testmsdp.msdpoff()
  log("OFF")
  msdp.unreport("ROOM")
  msdp.unreport("MOVEMENT")
--  msdp.unreport{'ROOM', 'MOVEMENT'}
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