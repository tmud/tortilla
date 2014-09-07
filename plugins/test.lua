-- пока только самые базовые функции без какой либо работы с данными
test = {}

function test.name() 
    return 'Тестовый плагин на LUA'
end

function test.description()
    return 'Проверка как это будет работать.'
end

function test.version()
    return '1.0'
end

function test.init()
    addMenu('Тестовая/Пункт 1', 1, 1)
    addCommand("test")
end

function test.menucmd(id)
    --tabs:add("123456")
    --littgroups:add("sssys")
    aliases:select(0)
    aliases:setindex(1)
end
