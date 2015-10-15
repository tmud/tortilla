system = require ('system')
rnd = require 'rnd'

local function prequire(m)
  local ok, mod = pcall(require, m) 
  if not ok then return nil, mod end
  return mod
end

local res, err
bass,err = prequire('lbass')
if not bass then
    system.msgbox(err, "error")
else
    res, err = bass.init()
    if not res then
        system.msgbox(err, "error")
    else
        regUnloadFunction(bass.free)
    end
end
