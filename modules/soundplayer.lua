-- модуль воспроизведения звуковых файлов
-- использует для своей работы модуль bass (lbass.dll + bass.dll)
-- модуль soundplayer используется плагином sound

if soundplayer then return end
dofile 'modules.lua'

local function print(...)
  log(...)
end

if not bass then
  print("Модуль SoundPlayer не загружен, требуется Bass.")
end

soundplayer = {}
local sp = {}

local function make_list()
  local path = getResource("")
  print(path)
end

function soundplayer.getVolume()
  return bass.getVolume()
end

function soundplayer.setVolume(v)
  return bass.setVolume(v)
end

function soundplayer.music(name, volume)
  if sp.music then
    bass.stop(sp.music)
  end  
  local id, err = bass.loadStream(name)
  if not id then
    print(err)
	return false
  end
  sp.music = id
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

local function unload()
  if sp.music then bass.unload(sp.music) end
end
regUnloadFunction(unload)
make_list()
