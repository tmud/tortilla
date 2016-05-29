-- Модуль отбора строк от ключевой строки до prompt.
-- Функция создает объект, который будет собирать все строки от ключевой до промпт строки.
-- Функция-фильтр (необязательно) позволяет отбирать нужные строки - запоминать или не запоминать.

--[[ Как использовать:

local function filter(vs)
  -- функция фильтр для отбора нужных строк, vs - это viewstring
  -- 1 результат - false - строку не сохранять в триггере, true - строку сохранить/запомнить.
  -- 2 результат - false - строку оставить в исходном окне, true - удалить (дропнуть) из исходного окна.
  return true, false
end

local t
function plugin.init()
  -- создаем триггер
  t = prompt_trigger('Вы используете:', filter)
end

function plugin.before(v, vd)
  -- отбор данных
  if t and t:сheck(vd) then
    -- триггер сработал, обработка
    local strings = t.strings
  end
end

function plugin.disconnect()
  -- при обрыве нужно сбросить триггер
  if t then
    t:disconnect()
  end
end
]]

function prompt_trigger(key_string, filter_function)
  local pcre = createPcre(key_string)
  if not pcre then
    return nil, '[prompt_trigger] Триггер не создан. Некорректное значение key_string.'
  end
  if filter_function and type(filter_function) ~= 'function' then
    return nil, '[prompt_trigger] Триггер не создан. filter_function - не функция.'
  end

  local trigger = { pcre = pcre, filter = filter_function, collect_mode = false, strings ={} }

  function trigger:check(vd)
    if not self.collect_mode then
      if vd:find(self.pcre) then
        self.collect_mode = true
        self.strings = {}
        local vs = vd:createRef()
        if self.filter then
          local ref,drop = self.filter(vs)
          if not ref then vs = nil end
          if drop then vd:dropString() end
        end
        if vs then
          local s = self.strings
          s[#s+1] = vs
        end
        local index,size = vd:getIndex(),vd:size()
        if index == size then
          return false
        end
        vd:select(index+1)
      else
        return false
      end
    end
    local index,size = vd:getIndex(),vd:size()
    for i=index,size do
      vd:select(i)
      if vd:isPrompt() then
        self.collect_mode = false
        return true
      end
      local vs = vd:createRef()
      if self.filter then
        local ref,drop = self.filter(vs)
        if not ref then vs = nil end
        if drop then vd:dropString() end
      end
      if vs then
        local s = self.strings
        s[#s+1] = vs
      end
    end
    return false
  end

  function trigger:disconnect()
    self.collect_mode = false
    self.strings = {}
  end

  return trigger
end