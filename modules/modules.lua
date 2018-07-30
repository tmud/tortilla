-- modules.lua
-- загрузчик dll-модулей

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

off = {}
local t = system.loadTextFile('../off.txt')
if not t then
  t = system.loadTextFile('../resources/off.txt')
  if t then
    system.saveTextFile('../off.txt', t)
  end
end
if t then
  for _,s in ipairs(t) do 
    if not s:contain('# ') then off[s] = true end
  end
end

extra = require 'extra'

local function prequire(m)
  local ok, mod = pcall(require, m)
  if not ok then return nil, mod end
  return mod
end

if not off.sound then
if not bass then
  local res, err
  bass,err = prequire('lbass')
  if not bass then
    print (err)
  else
    res, err = bass.init()
    if not res then
      print (err)
    else
      regUnloadFunction(bass.free)
    end
  end
end
end

if not off.voice then
lvoice,err = prequire ('voice')
if lvoice then
  if lvoice.init() then
    regUnloadFunction(lvoice.release)
  else
    lvoice = nil
    print("[lvoice] Ошибка при инициализации модуля.")
  end
else
  print("[lvoice] Ошибка при загрузке модуля: "..err)
end
end
