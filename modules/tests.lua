-- tests.lua
-- тестовый код для проверки функционала клиента и модулей, юнит тесты
-- запускаются при старте клиента

dofile 'modules.lua'

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
  local s = dlib:find(s1)
  if s ~= s2 then
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
