﻿-- Пример плагина на языке Lua для мад-клиента Tortilla
-- 1. Это текстовый файл в кодировке UTF8
-- 2. Весь код должен быть в рамках объектов-таблиц и функций, которые являются частью этой таблицы.

-- таблица для программного интерфейса плагина (для плагинов обязательно local)
local plugin = {}

-- Для плагина требуется реализовать обязательные функции (см. документацию):
function plugin.name() 
    return 'Тестовый плагин на LUA'
end

function plugin.description()
    return 'Описание плагина.'
end

function plugin.version()
    return '1.0'
end

-- Другие функции необязательны и их нужно писать только при необходимости
function plugin.init()
    -- инициализация, плагин загружается
end

function test.release()
    -- освобождение всех ресурсов, плагин выгружается
end

-- 3. Не рекомендуется писать код вне рамок функций, так как он выполнится сразу же в момент загрузки.
-- 4. Название файла плагина желательно должно совпадать с названием самого плагина.
-- 5. Повсеместно используйте ключевое слово local для переменных и таблиц. 
-- Без ключевого слова local переменная будет считаться глобальной и это может быть источником конфликтов между модулями.

local b = 10                -- локальная переменная модуля (доступна только в данном файле)
local helper = {}           -- локальная объект-таблица будет ограничена областью видимости (доступна только в данном файле)

return plugin               -- возвращаем таблицу плагина (его программного интерфейса).
