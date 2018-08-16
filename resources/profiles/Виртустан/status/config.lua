﻿-- Местоположение в окне клиента "top"(вверху)  или "bottom"(внизу)
position = 'bottom'

-- Количество панелей. Чем больше панелей, тем меньше информации вмещает каждая панель
count = 7

-- Триггер тикера, начало отсчета очередного тика
-- Ненужен тикер - нужно закомментировать эту строку
-- Через запятую в кавычках можно задавать несколько триггеров
ticker = { '^Минул час.' }

-- Количество секунд в одном тике
ticker_seconds = 120

-- В какую панель выводить тикер
ticker_window = 1

-- Цвет текста и фона в панели с тикером (#help color). Необязательно.
ticker_color = 'white'

-- Рестарт тикера, если не приходит ключевая фраза
ticker_restart = false

-- Триггеры тикера, за N секунд до окончания тика
ticker_triggers = 
{
    [10] = "г до конца тика 10 секунд" ,

}