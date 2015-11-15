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

function soundplayer.playfx(filename, volume)
  local v = volume and volume or 100
  if v < 0 or v > 100 then
    log("Ошибка: Допустимый диапазон громкости 0-100")
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

local playlist, pl_index, pl_volume

local function stopplaylist()
  pl_volume = nil
  pl_index = nil
  pl_volume = nil
end

local function nextfile()
  local last_track = #playlist
  if last_track == 0 then
    return
  end
  local filename = playlist[pl_index]
  pl_index = pl_index + 1
  return filename
end

local function nexttrack()
  local filename = nextfile()
  while filename do
    local id, err = bass.loadStream(filename)
    if not id then
      log(err)
    end
    local res,err = bass.play(id, v, nextfile)
    if not res then
      log(err)
    else
      sp.music = id
      return
    end
  end
  stopplaylist()
end

function soundplayer.playlist(t, volume)
  for _,f in ipairs(t) do
    log(f)
  end

  local v = volume and volume or 100
  if v < 0 or v > 100 then
    log("Ошибка: Допустимый диапазон громкости 0-100")
    return false
  end
  soundplayer.stop(-1)
  playlist = t
  pl_volume = volume
  pl_index = 1
  --nexttrack()
  return -1
end

function soundplayer.play(filename, volume)
  local v = volume and volume or 100
  if v < 0 or v > 100 then
    log("Ошибка: Допустимый диапазон громкости 0-100")
    return false
  end

  soundplayer.stop(-1)
  local id, err = bass.loadStream(filename)
  if not id then
    log(err)
    return false
  end
  sp.music = id
  local res,err = bass.play(id, v)
  if not res then
    log(err)
    return false
  end
  return id
end

function soundplayer.stop(id)
  if not id then return end
  if id == -1 then
    stopplaylist()
    id = sp.music
  end
  if id == sp.music then
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
