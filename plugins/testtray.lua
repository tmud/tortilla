-- testtray
-- Плагин Tortilla mud client

local testtray = {}
function testtray.name() 
    return 'Тестовый плагин для плагина tray'
end

function testtray.description()
    return 'Плагин используется для тестирования плагина tray.'
end

function testtray.version()
    return '-'
end

function testtray.syscmd(t)
    if t[1] ~= 'testtray' then
        return t
    end
	local r = rnd.rand(0,9)
    local cmd = props.cmdPrefix()..'tray ['..r..']:'..rnd.string(50, 140)
    runCommand(cmd)
    return false
end

return testtray