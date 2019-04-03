﻿-- Пример модуля на языке Lua для мад-клиента Tortilla
-- 1. Это текстовый файл в кодировке UTF8 ( с BOM маркером ).
-- 2. Весь код должен быть в рамках объектов-таблиц и функций, которые являются частью этой таблицы.

module = {}                 -- объект-таблица модуля (без local, чтобы модуль был виден отовсюду, это правило только для модулей).
function module.add(a, b)   -- функция из модуля
  return a+b
end

function module.sub(a, b)   -- следующая функция модуля
  return a-b
end

-- 3. Не рекомендуется писать код вне рамок функций, так как он выполнится сразу же в момент загрузки, 
-- и так как последовательность загрузки других модулей не гарантирована, то любые обращения в другие модули некорректны!
-- 4. Название файла модуля желательно должно совпадать с названием самого модуля, чтобы не путаться.
-- 5. Повсеместно используйте ключевое слово local для переменных и таблиц. Без ключевого слова local переменная будет считаться
-- глобальной и это потенциально будет источником конфликтов между модулями.

local b = 10                -- локальная переменная модуля (доступна только в данном файле)
local helper = {}           -- локальная объект-таблица (доступна только в данном файле)

-- 5. Вызов модуля из другого модуля или плагина, через обращение через имя модуля, пример из другого файла:
--function module2()
--    return module.add(10, 20)
--end

-- 6. Можно проверять наличие модуля так:
-- if module then
--   module.method(...)
-- end
