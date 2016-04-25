-- конфигурационный файл для плагина аффектов (affects.lua)
-- good - позитивные аффекты
-- bad - негативные аффекты

-- цвет аффектов, палитра 256 цветов
colors = { good_active=10, good_inactive=2, bad_active=9, bad_inactive=1 }

-- Аффекты, которые нужно отображать
good_affects = {
}

bad_affects = {
{ id = "thirst", name = "Жажда" },
{ id = "hunger", name = "Голод" },
}

triggers = {
{ key = "^Вы хотите пить.", on="thirst" },
{ key = "^Вы хотите есть.", on="hunger" },
}
