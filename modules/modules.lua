-- загрузчик dll-модулей
if system then return end
system = require 'system'
rnd = require 'rnd'

local function prequire(m)
  local ok, mod = pcall(require, m)
  if not ok then return nil, mod end
  return mod
end

if not bass then
  local res, err
  bass,err = prequire('lbass')
  if not bass then print (err)
  else
    res, err = bass.init()
    if not res then print (err) end
  end
end
