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
local sp = { samples = {} }

function soundplayer.getVolume()
  return bass.getVolume()
end

function soundplayer.setVolume(v)
  return bass.setVolume(v)
end

function soundplayer.play(filename, volume)
  local id, err = bass.loadSample(filename)
  if not id then
    print(err)
	return false
  end
  sp.samples[id] = true
  local v = volume and volume or 100
  if v > 100 then v = 100 end
  if v < 0 then v = 0 end
  local res,err = bass.play(id, v)
  if not res then
	 print(err)
	 return false
  end
  return id
end

function soundplayer.music(filename, volume)
  if sp.music then
    bass.stop(sp.music)
    bass.unload(sp.music)
	sp.music = nil
  end
  local id, err = bass.loadStream(filename)
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
  return id
end

function soundplayer.stop(id)
  if not id then return end
  if sp.music == id then 
    bass.stop(id)
    bass.unload(id)
	sp.music = nil
	return
  end
  bass.stop(id)
end

local function unload()
  if sp.music then
    bass.stop(sp.music)
    bass.unload(sp.music)
  end
  for id,_ in pairs(sp.samples) do
    bass.stop(id)
    bass.unload(id)
  end
  sp.samples = {}
end
regUnloadFunction(unload)

