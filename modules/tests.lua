-- tests.lua
-- тестовый код для проверки функционала модулей

--[[
dofile 'modules.lua'

local t = system.loadTextFile('c:\\tortilla\\mudclient\\readme.txt')
if not t then
  print ('file load error')
else
  for _,s in ipairs(t) do
    print (system.convertFromWin(s))
  end
end
]]
