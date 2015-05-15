-- update
-- Плагин Tortilla mud client

tupdate = {}
function tupdate.name() 
    return 'Тестовый плагин метода update для палагинов'
end

function tupdate.description()
    return 'Плагин используется для тестирования механизма оповещений.'
end

function tupdate.version()
    return ' '
end

function tupdate.update(what, pattern, del)
    if not what then
		log("Полное обновление")
		return
	end
	if not pattern then
		log("Обновление группы: "..what)
		return
	end
	local s = del and "УДАЛЕНО" or "ЗАДАНО"
	log("Обновление "..what..": "..pattern.."-"..s)
end
