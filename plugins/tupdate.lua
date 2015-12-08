-- update
-- Плагин Tortilla mud client

tupdate = {}
function tupdate.name() 
    return 'Тестовый плагин метода update для плагинов'
end

function tupdate.description()
    return 'Плагин используется для тестирования механизма оповещений.'
end

function tupdate.version()
    return ' '
end

function tupdate.update(what)
    if not what then
        log("Полное обновление")
    else
        log("Обновление группы: "..what)
    end
end

function tupdate.updset(what, name)
    log("ЗАДАНО "..what..": "..name)
end

function tupdate.upddel(what, name)
    log("УДАЛЕНО "..what..": "..name)
end
