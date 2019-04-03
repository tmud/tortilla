-- loadsave
-- Плагин Tortilla mud client

local loadsave = {}
function loadsave.name() 
  return 'Тесты loadTable, saveTable'
end
function loadsave.description()
  return 'Плагин используется для автотестирования функций loadTable, saveTable.'
end
function loadsave.version()
  return '-'
end

local function assert(res, text)
  if not res then
    print("unit test: "..text.." FAILED!")
  else
    print("unit test: "..text.." OK")
  end
end

local function test1()
  print('test1')
  local t = {
    mode = true,
    a = { 'проверка' },
    b = { 'проверка2', p = 4, b = false },
    c = { 'сложная проверка', d = { 'вложенная таблица' } }
  }
  saveTable(t, 'unittests.lua')
  local t2 = loadTable('unittests.lua')
  assert(t.mode == t2.mode, 't.mode == t2.mode')
  assert(t.a[1] == t2.a[1], 't.a[1] == t2.a[1]')
  assert(t.a[2] == t2.a[2], 't.a[2] == t2.a[2]')
  assert(t.b[1] == t2.b[1], 't.b[1] == t2.b[1]')
  assert(t.c[1] == t2.c[1], 't.c[1] == t2.c[1]')
  assert(t2.c.d[1] == 'вложенная таблица', 't2.c.d[1] == "вложенная таблица"')
  assert(t2.b.p == 4, 't2.b.p == 4')
  assert(t2.b.b == false, 't2.b.b == false')
  assert(t2.createPcre ~= nil, 't2.createPcre ~= nil')
  assert(t2.table ~= nil, 't2.table ~= nil')
  system.deleteFile(getPath('unittests.lua'))
  saveTable(t2, 'unittests2.lua')
  assert(t2.createPcre == nil, 't2.createPcre == nil')
  assert(t2.table == nil, 't2.table == nil')
  system.deleteFile(getPath('unittests2.lua'))
end

local b = { {["1"] = { exits={["w"]=10050, ["s"]=1001, array=200 },["name"]="фыва"}}, { "abc" }, ["3"] = 15, array = 100 }
local function test2(file)
  saveTable(b, file)
  local b2 = loadTable(file)
  local t1 = b[2]
  local t2 = b2[2]
  assert(t1 ~= nil and t2 ~= nil, "t1[2],t2[2] ~= nil")
  assert(t1[1] == t2[1], "t1[2][1] == t2[2][1]")
  t1 = b[2]
  t2 = b2[2]
  assert(t1 ~= nil and t2 ~= nil, "t1[2],t2[2] ~= nil")
  assert(t1[1] == t2[1], "t1[2][1] == t2[2][1]")
  t1 = b[1]
  t2 = b2[1]
  assert(t1 ~= nil and t2 ~= nil, "t1,t2 ~= nil")
  assert(t1["1"] ~= nil and t2["1"] ~= nil, 't1["1"],t2["1"] ~= nil')  
  t1 = t1["1"]
  t2 = t2["1"]
  assert(t1.exits ~= nil and t2.exits ~= nil, "t1.exits,t2.exits ~= nil")
  assert(t1.name == t2.name, "t1.name == t2.name")
  t1 = t1.exits
  t2 = t2.exits
  for k,v in pairs(t1) do
    local text = "t1."..k.." == t2."..k
    assert(v == t2[k], text)
  end
  t1 = b["3"]
  t2 = b2["3"]
  assert(t1 ~= nil and t2 ~= nil, "t1['3'],t2['3'] ~= nil")
  assert(t1 == t2, "t1['3'] == t2['3']")
  t1 = b["array"]
  t2 = b2["array"]
  assert(t1 ~= nil and t2 ~= nil, "t1['array'],t2['array'] ~= nil")
  assert(t1 == t2, "t1['array'] == t2['array']")
  system.deleteFile(getPath(file))
end

local function test2_lua()
  print('test2_lua')
  test2('unittests3.lua')
end

local function test2_xml()
  print('test2_xml')
  test2('unittests3.xml')
end

function loadsave.init()
  log("Автотесты для loadTable, saveTable")
  test1()
  test2_lua()
  test2_xml()
  runCommand("#wait 2 { #plugin testloadsave off }")
end
return loadsave
