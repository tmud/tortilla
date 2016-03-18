-- конфигурационный файл для плагина Инвентаря и Экипировки

-- цвет
colors = { header=80, tegs=150, equipment=180, inventory=180 }

-- слоты, которые нужно отслеживать, задает порядок отображения
slots = {
{ id="head", name="Голова" },
{ id="eyes", name="Глаза" },
{ id="lear", name="Ухо" },
{ id="rear", name="Ухо" },
{ id="neck", name="Шея" },
{ id="chest", name="Грудь" },
{ id="body", name="Тело" },
{ id="shoulders", name="Плечи" },
{ id="waist", name="Талия" },
{ id="hands", name="Руки" },
{ id="lwrist", name="Запястье" },
{ id="rwrist", name="Запястье" },
{ id="brushes", name="Кисти" },
{ id="lfinger", name="Палец" },
{ id="rfinger", name="Палец" },
{ id="legs", name="Ноги" },
{ id="boots", name="Обувь" },
{ id="shield", name="Щит" },
{ id="lhand", name="Левая" },
{ id="rhand", name="Правая" },
{ id="light", name="Свет" },
}

-- триггеры на одевание
dress={
{ key="Вы надели %1 на голову.", id="head" },
{ key="Вы надели %1 на тело.", id="body" },

}

-- триггеры на раздевание
undress={
{ key="Вы прекратили использовать %1." },
}

