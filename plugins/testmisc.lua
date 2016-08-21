-- testmisc
-- Плагин для Tortilla mud client

local testmisc = {}
function testmisc.name()
  -- плагин отключен
  return 'Тесты viewdata'
end
function testmisc.version()
  return '-'
end
function testmisc.description()
  return "Плагин для тестирования viewdata"
end

function testmisc.init()
--addCommand('testvd')
end

local flag = false
function testmisc.before(v, vd)
  if v ~= 0 then return end
  if flag then return end
  log('before')
  flag = true
  vd:select(0)
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
end

function testmisc.syscmd(t)
  local c = t[1]
  if c == 'testvd' then
  end
  return t
end

return testmisc
