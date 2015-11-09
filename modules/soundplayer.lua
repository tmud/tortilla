-- модуль воспроизведения звуковых файлов
-- использует для своей работы модуль bass (lbass.dll + bass.dll)
-- модуль soundplayer используется плагином sound

local function print(...)
  log(...)
end

local bass
local function prequire(m)
  local ok, mod = pcall(require, m)
  if not ok then return nil, mod end
  return mod
end

local res, err
bass,err = prequire('lbass')
if not bass then
  print ("Модуль Bass не загружен: "..err)
else
  res, err = bass.init()
  if not res then
    print ("Модуль Bass не загружен: "..err)
  end
end

if not bass then
  print("Модуль SoundPlayer не загружен, требуется Bass.")
end

soundplayer = {}
function soundplayer.unload()
  if soundplayer.music then bass.unload(soundplayer.music) end
  bass.free()
end

function soundplayer.getVolume()
  return bass.getVolume()
end

function soundplayer.setVolume(v)
  return bass.setVolume(v)
end

function soundplayer.music(name, volume)
  if soundplayer.music then
    bass.stop(soundplayer.music)
  end  
  local id, err = bass.loadStream(name)
  if not id then
    print(err)
	return false
  end
  soundplayer.music = id
  local v = volume and volume or 100
  if v > 100 then v = 100 end
  if v < 0 then v = 0 end
  local res,err = bass.play(id, v)
  if not res then
	 print(err)
	 return false
  end
  return true
end

regUnloadFunction(soundplayer.unload)
