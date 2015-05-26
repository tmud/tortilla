local function prequire(m)
  local ok, err = pcall(require, m) 
  if not ok then return nil, err end
  return err
end

system = require ('system')
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

local n = bass.load('d:\\1.mp3')
local m = bass.load('d:\\2.mp3')
bass.play(m)
bass.play(n)

--local x = bass.loadSample('d:\\sample.wav')
--bass.play(x)

--local v = bass.getVolume()
system.msgbox(false)
--bass.setVolume(50)