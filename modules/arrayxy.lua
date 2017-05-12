-- Класс двумерного массива
-- Использование (индексы начинаются с 1, т.к. Lua)
-- local a = array_class:new(10, 5)  -- создание массива 10 на 5
-- a:set(1,3, 100) - запись в массив [1,3] = 100
-- local x = a:get(1,3) - получение из массива [1,3]

array_class = {}
function array_class:new(w, h)
  local array = { width = w, height = h }
  function array:index(x, y)
    if x >= 1 and x <= self.width and y >= 1 and y <= self.height then
      return self.width*(y-1) + x
    end
  end
  function array:logerr(x, y, method)
    log('Выход за пределы массива arrayxy:'..method..' index={'..x..','..y..'}, size={'..self.width..','..self.height..'}')
  end
  return make_class_object(array_class, array)
end

function array_class:get(x, y)
  local i = self:index(x, y)
  if i then
    return self[i]
  end
  self:logerr(x, y, 'get')
end

function array_class:set(x,y,v)
  local i = self:index(x, y)
  if i then
    self[i] = v
    return
  end
  self:logerr(x, y, 'set')
end
