-- modules.lua
-- загрузчик модулей

if modules then return end
modules = "loaded"

function pairsByKeys (t, f)
  local a = {}
  for n in pairs(t) do table.insert(a, n) end
  table.sort(a, f)
  local i = 0      -- iterator variable
  local iter = function ()   -- iterator function
    i = i + 1
    if a[i] == nil then return nil
    else return a[i], t[a[i]]
    end
  end
  return iter
end

function make_class_object(class, object)
  object = object or {}
  local interface = {}
  for k,v in pairs(class) do
    if k ~= 'new' then
      local f = class[k]
      interface[k] = function(p, ...)
        if p == interface then
          return f(object, ...)       -- object:method call
        else
          return f(object, p, ...)    -- object.method call
        end
      end
    end
  end
  setmetatable(object, { __index = class })
  return setmetatable(interface, {} )
end

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

local function prequire(m)
  local ok, mod = pcall(require, m)
  if not ok then return nil, mod end
  return mod
end

local res, err
bass,err = prequire('lbass')
if not bass then
  print (err)
else
  res, err = bass.init()
  if not res then
    print (err)
    bass = nil
  else
    regUnloadFunction(bass.free)
  end
end

lvoice,err = prequire ('voice')
if lvoice then
  if lvoice.init() then
    regUnloadFunction(lvoice.release)
  else
    print("[lvoice] Ошибка при инициализации модуля.")
    lvoice = nil
  end
else
  print("[lvoice] Ошибка при загрузке модуля: "..err)
end
