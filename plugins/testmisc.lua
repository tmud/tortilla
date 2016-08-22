-- testmisc
-- Плагин для Tortilla mud client

local testmisc = {}
function testmisc.name()
  -- плагин отключен
  return 'Тесты viewdata и viewstring'
end
function testmisc.version()
  return '-'
end
function testmisc.description()
  return "Плагин для тестирования viewdata и viewstring"
end

function testmisc.init()
--addCommand('testvd')
end

local flag = false
function testmisc.before(v, vd)
  if v ~= 0 then return end
  if flag then return end
  flag = true
  vd:select(1)
  log(vd:getText())
  vd:deleteAllBlocks()
  vd:setBlocksCount(2)
  vd:setBlockText(1, "Test")
  vd:setBlockText(2, "Block")
  vd:set(1, 'textcolor', 4)
  vd:set(2, 'textcolor', 6)
  local p = vd:insertBlock(2)
  vd:setBlockText(p, '123')
  vd:set(p, 'textcolor', 5)
  local p2 = vd:insertBlock(8)
  vd:setBlockText(p2, 'xyz')
  vd:set(p2, 'textcolor', 2)
  local p3 = vd:insertBlock(11)
  vd:setBlockText(p3, '456')
  
  local vs = createViewString()
  vs:setBlocksCount(1)
  vs:setBlockText(1, 'Test vs')
  vs:print(0)
  local v0 = vs:insertBlock(8)
  vs:setBlockText(v0, 'end')
  vs:set(v0, 'textcolor', 3)
  
  local v1 = vs:insertBlock(4)
  vs:setBlockText(v1, ' ins')
  vs:set(v1, 'textcolor', 2)
  vs:print(0)
  local v2 = vs: insertBlock(8)
  vs:setBlockText(v2, 'abc')
  vs:set(v2, 'textcolor', 6)
  vs:print(0)
  
  vd:print(0)
  --local p4 = vd:insertBlock(20)
  
  local p3 = vd:insertBlock(19)
  vd:setBlockText(p3, 'qwe')
  vd:setBlockText(1, 'vd_text')
  
  local vs2 = createViewString()
  vs2:setBlocksCount(3)
  vs:copyBlock(2, vs2, 1)
  vs:copyBlock(3, vs2, 2)
  vs:copyBlock(5, vs2, 3)
  vs:copyBlock(4, vd, 2)
  vs2:print(0)
  
end

function testmisc.syscmd(t)
  local c = t[1]
  if c == 'testvd' then
  end
  return t
end

return testmisc
