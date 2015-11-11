-- soundplayer.lua
-- модуль воспроизведения звуковых файлов
-- использует для своей работы bass (lbass.dll + bass.dll)
-- модуль soundplayer используется плагином sound

if soundplayer then return end

dofile 'modules.lua'

local function log(...)
  print('[soundplayer]', ...)
end

if not bass then
  log("Модуль SoundPlayer не загружен, требуется Bass.")
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
  local v = volume and volume or 100
  if v < 0 or v > 100 then
    log("Ошибка: Допустимый диапозон громкости 0-100")
	return false
  end
  local id, err = bass.loadSample(filename)
  if not id then
    log(err)
	return false
  end
  sp.samples[id] = true
  local res,err = bass.play(id, v)
  if not res then
	 log(err)
	 return false
  end
  return id
end

function endplaying(id)
  log(id)
end

function soundplayer.music(filename, volume)
  local v = volume and volume or 100
  if v < 0 or v > 100 then
    log("Ошибка: Допустимый диапозон громкости 0-100")
	return false
  end
  if sp.music then
    bass.stop(sp.music)
    bass.unload(sp.music)
	sp.music = nil
  end
  local id, err = bass.loadStream(filename)
  if not id then
    log(err)
	return false
  end
  sp.music = id
  local res,err = bass.play(id, v, endplaying)
  if not res then
	 log(err)
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

