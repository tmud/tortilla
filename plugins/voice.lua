-- voice
-- Плагин для Tortilla mud client

voice = {}
function voice.name() 
  return 'Синтезатор речи'
end

function voice.description()
  local s = 'Плагин синтезирует текст в речь. Подробно в справке #help voice.\r\n'
  s = s..'Плагин использует программный интерфейс SAPI5.\r\n'
  s = s..'Все установленные голоса на базе SAPI5 могут использоваться плагином.\r\n'
  s = s..'#voice list - список установленных голосов на компьютере\r\n'
  s = s..'#voice select [номер] - текущий голос/выбрать голос\r\n'
  s = s..'#voice speak текст - прочитать текст\r\n'
  s = s..'#voice stop - остановить чтение текста\r\n'
  s = s..'#voice volume [громкость] - текущая громкость/задать громкость [0,100]\r\n'
  s = s..'#voice rate [темп] - текущий темп речи/задать темп речи [-10,10]'
  return s
end

function voice.version()
    return '1.0'
end

local function isnumber(s)
  if type(s) == 'number' then return true end
  if type(s) ~= 'string' then return false end
  return tonumber(s) or true and false
end

local function print(...)
  _G.print('[voice]', ...)
end

local current_voice
function voice.init()
  if not lvoice then
    return terminate("Модуль lvoice не загружен")
  end
  addCommand('voice')
  local t = loadTable('config.xml')
  if t then
    if isnumber(t.volume) then
      local v = tonumber(t.volume)
      if v >= 0 and v <= 100 then lvoice.setVolume(v) end
    end
    if isnumber(t.rate) then
      local r = tonumber(t.rate)
      if r >= -10 and r <= 10 then lvoice.setRate(r) end
    end
    if t.voice then
      local l = lvoice.getVoices()
      if type(l) == 'table' then
        for id,v in ipairs(l) do
          if v == t.voice then lvoice.selectVoice(id); current_voice = v; break; end
        end
      end
    end
  end
end

function voice.release()
  if not lvoice then return end
  local t = { volume = lvoice.getVolume(), rate  = lvoice.getRate(), voice = current_voice }
  saveTable(t, 'config.xml')
end

function voice.syscmd(t)
  local c = t[1]
  if c == 'voice' then
    local op = t[2]
    if op == 'speak' then
      if not current_voice then
        print('Не задан голос')
        return
      end
      local text = {}
      for i=3,#t do
        text[i-2] = t[i]
      end
      lvoice.speak(table.concat(text, ' '))
      return
    elseif op == 'stop' then
      if #t == 2 then
        lvoice.stop()
        return
      end
    elseif op == 'volume' then
      if #t == 3 and isnumber(t[3]) then
        local v = tonumber(t[3])
        if v >= 0 and v <= 100 then
          print('Новая громкость: '..v)
          lvoice.setVolume(v)
          return
        end
      elseif #t == 2 then
        local v = lvoice.getVolume()
        print('Текущая громкость: '..v)
        return
      end
    elseif op == 'rate' then
      if #t == 3 and isnumber(t[3]) then
        local v = tonumber(t[3])
        if v >= -10 and v <= 10 then
          print('Новый темп речи: '..v)
          lvoice.setRate(v)
          return
        end
      elseif #t == 2 then
        local v = lvoice.getRate()
        print('Текущий темп речи: '..v)
        return
      end
    elseif op == 'list' then
      if #t == 2 then
        local l = lvoice.getVoices()
        if type(l) == 'table' then
          print('Список голосов:')
          for id,v in ipairs(l) do
            local s = id..':'..v
            if v == current_voice then s = s..' <<<' end
            _G.print(s)
          end
        else
          print('Ошибка получения списка голосов')
        end
        return
      end
    elseif op == 'select' then
      if #t == 2 then
        if current_voice then
          print('Текущий голос: '..current_voice)
        else
          print('Голос не выбран')
        end
        return
      end
      if #t == 3 and isnumber(t[3]) then
        local v = tonumber(t[3])
        local l = lvoice.getVoices()
        if type(l) == 'table' and v >= 1 and v <= #l and lvoice.selectVoice(v) then
          current_voice = l[v]
          print('Выбран голос: '..l[v])
          return
        end
      end
    end
    return 'Неверный набор параметров'
  end
  return t
end
