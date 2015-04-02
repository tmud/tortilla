-- testmsdp
-- Плагин Tortilla mud client

testmsdp = {}
function testmsdp.name() 
    return 'Тестовый плагин MSDP'
end

function testmsdp.description()
    return 'Плагин используется для тестирования протокола MSDP.'
end

function testmsdp.version()
    return '-'
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
    --msdp.list("REPORTABLE_VARIABLES")
    msdp.report("ROOM")
end

function testmsdp.msdp(t)
    log("data")
end
