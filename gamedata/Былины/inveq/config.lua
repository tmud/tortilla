-- конфигурационный файл для плагина Инвентаря и Экипировки

-- цвет текста, палитра 256 цветов
colors = { header=80, tegs=150, equipment=180, inventory=180 }

-- рисовать (true), не рисовать (false) имена (name) слотов в экипировке
slots_name = true

-- слоты, которые нужно отслеживать, задает порядок отображения
slots = {
{ id="head", name="Голова" },
{ id="eyes", name="Глаза" },
{ id="rear", name="Ухо" },
{ id="lear", name="Ухо" },
{ id="neck", name="Шея" },
{ id="chest", name="Грудь" },
{ id="back", name="ЗаСпиной" },
{ id="body", name="Тело" },
{ id="shoulders", name="Плечи" },
{ id="waist", name="Талия" },
{ id="hands", name="Руки" },
{ id="rwrist", name="Запястье" },
{ id="lwrist", name="Запястье" },
{ id="wrists", name="Кисти" },
{ id="rfinger", name="Палец" },
{ id="lfinger", name="Палец" },
{ id="legs", name="Ноги" },
{ id="boots", name="Обувь" },
{ id="shield", name="Щит" },
{ id="rhand", name="Правая" },
{ id="lhand", name="Левая" },
{ id="inhands", name="Обе руки" },
{ id="light", name="Свет" },
}

-- таблица для команды экипировка
local eqtable = {
  ["надето на глаза"] = "eyes",
  ["надето на голову"] = "head",
  ["в правом ухе"] = "rear",
  ["в левом ухе"] = "lear",
  ["надето на шею"] = "neck",
  ["надето на грудь"] = "chest",
  ["за спиной"] = "back",
  ["надето на тело"] = "body",
  ["наброшено на плечи"] = "shoulders",
  ["надето на талии"] = "waist",
  ["надето на руки"] = "hands",
  ["на правом запястье"] = "rwrist",
  ["на левом запястье"] = "lwrist",
  ["надето на кисти рук"] = "wrists",
  ["надето на палец правой руки"] = "rfinger",
  ["надето на палец левой руки"] = "lfinger",
  ["надето на ноги"] = "legs",
  ["используется как обувь"] = "boots",
  ["используется как щит"] = "shield",
  ["в правой руке"] = "rhand",
  ["в левой руке"] = "lhand",
  ["в обеих руках"] = "inhands",
  ["для освещения"] = "light"
}

-- триггеры на одевание в процессе игры
-- обязательно указывать id слота
dress={
{ key="^Вы одели %1 на глаза.", id="eyes" },
{ key="^Вы надели %1 на голову.", id="head" },
{ key="^Вы вставили %1 в свое правое ухо.", id="rear" },
{ key="^Вы вставили %1 в свое левое ухо.", id="lear" },
{ key="^Вы надели %1 вокруг шеи.", id="neck" },
{ key="^Вы надели %1 на грудь.", id="chest" },
{ key="^Вы одели %1 за спину.", id="back" },
{ key="^Вы надели %1 на тело.", id="body" },
{ key="^Вы накинули %1 на плечи.", id="shoulders" },
{ key="^Вы надели %1 вокруг своей талии.", id="waist" },
{ key="^Вы одели %1 на свои руки.", id="hands" },
{ key="^Вы надели %1 на правое запястье.", id="rwrist" },
{ key="^Вы надели %1 на левое запястье.", id="lwrist" },
{ key="^Вы надели %1 на кисти рук.", id="wrists" },
{ key="^Вы надели %1 на палец правой руки.", id="rfinger" },
{ key="^Вы надели %1 на палец левой руки.", id="lfinger" },
{ key="^Вы одели %1 на ноги.", id="legs" },
{ key="^Вы обулись в %1.", id="boots" },
{ key="^Вы начали использовать %1 как щит.", id="shield" },
{ key="^Вы вооружились %1.", id="rhand" },
{ key="^Вы взяли %1 в левую руку.", id="lhand" },
{ key="^Вы взяли %1 в обе руки.", id="inhands" },
{ key="^Вы засветили %1 и взяли во вторую руку.", id="light" },
-- команда экипировка
{ key="^На вас надето:", ip = true,
  id = function(create_pcre)
    local regexp = create_pcre("^<(.*)> +(.*?) +[<(]")
    local table = eqtable
    return function(s)
      if regexp:find(s) then
        local slot = table[regexp:get(1)]
        if slot then return regexp:get(2), slot end
      end
    end
  end
},
}

-- триггеры на раздевание, предмет попадает в инвентарь
-- если id не указан, то идет поиск по слотам
undress={
{ key="Вы прекратили использовать %1." },
}

-- триггеры для добавление в инвентарь (кроме случаев одевания/снятия экипировки)
inventory_in = {
{ key = "^Вы забрали со склада %1." },
{ key = "^Вы взяли %1." },
{ key = "^Вы купили %1." },
{ key = "Вы сняли '%1' с аукциона." },
{ key = "^Вы несете:", ip = true, clear = true,  -- clear - очистка списка инветаря
  id = function(create_pcre)
    local regexp = create_pcre("^(?:\\([У|у]\\))?([^0-9\\[]+?)(?: +)\\[(.*)\\]")
    local regexp2 = create_pcre("^(?:\\([У|у]\\))?(.*)")
    return function(s)
      if s == " Ничего." then return "", 1 end
      if regexp:find(s) then
        return regexp:get(1), regexp:get(2)
      end
      if regexp2:find(s) then
        return regexp2:get(1), 1
      end
      return s, 1
    end
  end
},
{ key = "^Вы забрали со склада, нажитые непосильным трудом, следующие предметы:", ip = true,
  id = function()
    return function(s)
      return s, 1
    end
  end
},
}

-- триггеры для удаления из инвентаря (кроме случаев одевания/снятия экипировки)
inventory_out = {
{ key = "^Вы сдали %1 на склад." },
{ key = "^Вы бросили %1." },
{ key = "^Вы съели %1." },
{ key = "^%% купил%% у Вас %1." },
{ key = "^Вы дали %1 %%." },
{ key = "Вы выставили на аукцион %1 за %%" },
{ key = "^Вы сдали на склад следующие предметы:",
  id = function(create_pcre)
    local regexp = create_pcre("[0-9]+. *(.*).")
    return function(s)
      if regexp:find(s) then
        return regexp:get(1)
      end
    end
  end
},
}
