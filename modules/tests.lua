-- tests.lua
-- тестовый код для проверки функционала клиента и модулей, юнит тесты
-- запускаются при старте клиента

do return end

dofile 'modules.lua'

local maj,min = getVersion()
print("Версия клиента: "..maj.."."..min)
print ('Автотесты для модулей extra.declension и extra.dictonary')

local dlib = extra.declension()

local function assert(s1, s2, r)
  local result = dlib:compare(s1, s2)
  if result ~= r then
    print("unit test (similar) faled: ["..s1.."],["..s2.."]")
  end
end

assert('золотая монета', ' золотую   монету ', true)
assert('синяя палка', ' синюю   палку ', true)
assert('кушак повара', 'кушаком повара', true)
assert('зеленая миска', 'зеленой миской', true)
assert('оловянная ложка', 'оловянной ложке', true)
assert('змеиную маску', 'змеиная маска', true)

dlib:add('золотая монета')
dlib:add('синяя палка')
dlib:add('кушак повара')
dlib:add('зеленая миска')
dlib:add('змеиная маска')
dlib:add('ржавый меч')
dlib:add('тяжелый камень')
dlib:add('сачек бабочки')

local sorted = {
'зеленая миска',
'змеиная маска',
'золотая монета',
'кушак повара',
'ржавый меч',
'сачек бабочки',
'синяя палка',
'тяжелый камень'
}

for i,s in ipairs(sorted) do
  local result = dlib:check(s, i)
  if not result then
    print("unit test (sorting) faled: ["..s.."]:"..tostring(i))
  end
end

local function assert2(s1, s2)
  local s = dlib:similar(s1)
  if #s ~= 1 or s[1] ~= s2 then
     print("unit test (find) faled: ["..s1.."]:["..s2.."]")
  end
end

assert2('зеленую миску', 'зеленая миска')
assert2('зеленой миской', 'зеленая миска')
assert2('зеленое миске', 'зеленая миска')

assert2('змеиной маской', 'змеиная маска')
assert2('змеиной маске', 'змеиная маска')
assert2('змеиная маска', 'змеиная маска')

assert2('кушаку повара', 'кушак повара')
assert2('кушаком повара', 'кушак повара')
assert2('кушаке повара', 'кушак повара')

assert2('сачка бабочки', 'сачек бабочки')
assert2('сачку бабочки', 'сачек бабочки')
assert2('сачке бабочки', 'сачек бабочки')

dlib:clear()
dlib = nil

local dict = extra.dictonary(".\\")
local function dadd(s)
  dict:add(s, 'Инфа: '..s)
end

dadd('зеленая миска')
dadd('змеиная маска')
dadd('золотая монета')
dadd('кушак повара')
dadd('сачек бабочки')
dadd('ржавый меч')
dadd('длинный меч удали')
dadd('руна движения')
dadd('клыки крылатого огненного змея')
dadd('боярский кафтан')
dadd('круглый дубовый щит')
dadd('перстень с мутным камнем')
dadd('перстень света')


local function assert3(str, t)
  local r = dict:find(str)
  local count = 0
  for k,_ in pairs(r) do
    --print(k)
    count = count + 1
  end
  if count ~= #t then
    print("unit test (dictonary) faled: ["..str.."]")
    return
  end
  for s,_ in pairs(r) do
    local found = false
    for _,s2 in ipairs(t) do
      if s == s2 then found = true break end  
    end
    if not found then
      print("unit test (dictonary) faled: ["..str.."]")
      return
    end
  end
end

assert3('з м', {'змеиная маска', 'зеленая миска', 'золотая монета' } )
assert3('к п', {'кушак повара' } )
assert3('рж м', {'ржавый меч' } )
assert3('к', {'кушак повара', 'боярский кафтан', 'клыки крылатого огненного змея', 'круглый дубовый щит', 'перстень с мутным камнем'} )
assert3('п св', {'перстень света'} )

dict:wipe()
dict = nil
print ('Автотесты для модулей extra.declension и extra.dictonary - КОНЕЦ')

-- автотесты для плагинов
runCommand("#wait 2 { #plugin loadsave on }")
