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
  print('viewdata and viewstring unit tests')
end

local function assert(s1, s2, m)
  if s1 ~= s2 then
    log("unit test ("..m..") faled: ["..tostring(s1).."],["..tostring(s2).."]")
  end
end

local flag = false
function testmisc.before(v, vd)
  if v ~= 0 then return end
  if flag then return end
  flag = true
  vd:select(1)
  vd:set(1, 'textcolor', 6)
  vd:print(0)
  vd:deleteAllBlocks()
  assert(vd:blocks(), 0, 'vd:blocks==0')
  vd:setBlocksCount(2)
  assert(vd:blocks(), 2, 'vd:blocks==2')
  vd:setBlockText(1, "Test")
  vd:setBlockText(2, "Block")
  assert(vd:getBlockText(1), 'Test', 'vd:getBlockText==Test')
  vd:set(1, 'textcolor', 4)
  vd:set(2, 'textcolor', 6)
  assert(vd:blocks(), 2, 'vd:blocks==2_2')
  assert(vd:get(2, 'textcolor'), 6, 'vd:get==6')
  vd:print(0)
  local p = vd:insertBlock(2)
  vd:setBlockText(p, '123')
  vd:set(p, 'textcolor', 5)
  assert(vd:blocks(), 4, 'vd:blocks==4')
  vd:print(0)
  local p2 = vd:insertBlock(8)
  vd:setBlockText(p2, 'xyz')
  vd:set(p2, 'textcolor', 2)
  assert(vd:blocks(), 5, 'vd:blocks==5')
  assert(vd:get(4, 'textcolor'), 2, 'vd:get==2')
  vd:print(0)
  local p3 = vd:insertBlock(11)
  vd:setBlockText(p3, '456')
  vd:print(0)
  assert(vd:blocks(), 6, 'vd:blocks==6')
  local p4 = vd:insertBlock(19)
  vd:setBlockText(p4, 'qwe')
  vd:setBlockText(1, 'vd_text')
  vd:print(0)
  assert(vd:blocks(), 7, 'vd:blocks==7')
  assert(vd:getText(),  'vd_text123estxyz456Blockqwe', 'vd:getText')
  
  vd:set(1, 'italic', 1)
  vd:set(2, 'bkgcolor', 5)
  vd:set(3, 'underline', 1)
  vd:set(4, 'blink', 1)
  vd:set(5, 'reverse', 1)
  vd:set(6, 'exttextcolor', 123423)
  vd:set(7, 'extbkgcolor', 123423)

  local vs = createViewString()
  vs:setBlocksCount(1)
  vs:setBlockText(1, 'test viewstring')
  vs:print(0)
  local v = vs:insertBlock(16)
  vs:setBlockText(2, 'end')
  vs:set(2, 'textcolor', 2)
  assert(vs:blocks(), 2, 'vs:blocks==2')
  
  local v0 = vs:insertBlock(8)
  vs:setBlockText(v0, 'abc')
  vs:set(v0, 'textcolor', 3)
  vs:print(0)
  assert(vs:blocks(), 4, 'vs:blocks==4')
  
  local v1 = vs:insertBlock(11)
  vs:setBlockText(v1, ' ins')
  vs:set(v1, 'textcolor', 2)
  vs:print(0)
  assert(vs:blocks(), 5, 'vs:blocks==5')
  
  local v2 = vs: insertBlock(5)
  vs:setBlockText(v2, 'xyz')
  vs:set(v2, 'textcolor', 5)
  vs:print(0)
  assert(vs:blocks(), 7, 'vs:blocks==7')
  assert(vs:getText(),  'testxyz viabc insewstringend', 'vs:getText')
  
  local vs2 = createViewString()
  vs2:setBlocksCount(7)
  vs2:setBlockText(1, 'b1-')
  vs2:set(2, 'bkgcolor', 3)
  vs2:setBlockText(2, 'b2-')
  vs2:set(3, 'italic', 1)
  vs2:setBlockText(3, 'b3-')
  vs2:set(4, 'underline', 1)
  vs2:setBlockText(4, 'b4-')
  vs2:set(5, 'blink', 1)
  vs2:setBlockText(5, 'b5-')
  vs2:set(6, 'reverse', 1)
  vs2:setBlockText(6, 'b6-')
  vs2:set(7, 'textcolor', 1)
  vs2:setBlockText(7, 'b7-')
  vs2:print(0)

  vs:copyBlock(2, vs2, 1)
  vs2:print(0)
  assert(vs2:getBlockText(1), 'xyz', 'vs:getBlockText(1)')
  assert(vs2:get(1, 'textcolor'), 5, 'vs:textcolor=5')
  
  vs:copyBlock(7, vs2, 7)
  vs2:print(0)
  assert(vs2:getBlockText(7), 'end', 'vs:getBlockText(7)')
  assert(vs2:get(7, 'textcolor'), 2, 'vs:textcolor=2')
  vs2:copyBlock(5, vs, 3)
  vs:print(0)
  
  vs:copyBlock(3, vd, 2)
  vs2:copyBlock(6, vd, 3)
  vd:print(0)
  assert(vd:getText(),  'vd_textb5-b6-xyz456Blockqwe', 'vs:getText')
  
  vd:copyBlock(1, 1, 5)
  vd:print(0)
  assert(vd:getBlockText(5), 'vd_text', 'vd:text=vd_text')

  vd:copyBlock(6, vs, 1)
  vs:print(0)
  assert(vs:getBlockText(1), 'Block', 'vs:text=Block')
  
  local c1 = vs:getBlockColor(1)
  log(c1)
  assert(c1, '31,226,1;64,64,64;;', 'vs:getBlockColor')
  
  local c2 = vd:getBlockColor(4)
  log(c2)
  vs:setBlockColor(2, c2)
  vs:print(0)
  
  vd:setBlockColor(1, c1)

  local s = 'Абра @кадабра'
  local p = s:find('@')
  assert(p, 6, 'string:find 1')
  
  local p2 = s:find('бр')
  assert(p2, 2, 'string:find 2')
  
  local p3 = s:find('бр', p2+1)
  assert(p3, 11, 'string:find 3')

end

return testmisc
