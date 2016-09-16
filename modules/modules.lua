-- modules.lua
-- загрузчик dll-модулей

if system then return end
system = require 'system'

off = {}
local t = system.loadTextFile('../off.lst')
if t then
  for _,s in ipairs(t) do 
    if not s:contain('# ') then off[s] = true end
  end
end

rnd = require 'rnd'
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
