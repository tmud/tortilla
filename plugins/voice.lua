-- voice
-- Плагин для Tortilla mud client

voice = {}
function voice.name() 
  return 'Синтезатор речи'
end

function voice.description()
  local s = 'Плагин синтезирует текст в речь\r\n'
  s = s..'Используется программный интерфейс SAPI5.\r\n'
  s = s..'Все синтезаторы голоса на базе SAPI5 могут использоваться плагином.\r\n'
  s = s..'#voice list - список установленных голосов\r\n'
  s = s..'#voice select номер - выбрать голос\r\n'
  return s
end

function voice.version()
    return '1.0'
end

function voice.init()
  addCommand('voice')
end

local function isnumber(s)
  if type(s) == 'number' then return true end
  if type(s) ~= 'string' then return false end
  return tonumber(s) or true and false
end

function voice.syscmd(t)
  local c = t[1]
  if c == 'voice' and #t > 1 then
    local op = t[2]
    if op == 'play' then
      local text = {}
      for i=3,#t do
        text[i-2] = t[i]
      end
      lvoice.play(table.concat(text, ' '))
      return
    elseif op == 'stop' then
      if #t == 1 then
        lvoice.stop()
        return
      end
    elseif op == 'volume' then
      if #t == 3 and isnumber(t[3]) then
        local v = tonumber(t[3])
        if v >= 0 and v <= 100 then
          print('[voice] Новая громкость: '..v)
          lvoice.setVolume(v)
          return
        end
      elseif #t == 2 then
        local v = lvoice.getVolume()
        print('[voice] Текущая громкость: '..v)
        return
      end
    elseif op == 'rate' then
      if #t == 3 and isnumber(t[3]) then
        local v = tonumber(t[3])
        if v >= -10 and v <= 10 then
          print('[voice] Новый темп речи: '..v)
          lvoice.setRate(v)
          return
        end
      elseif #t == 2 then
        local v = lvoice.getRate()
        print('[voice] Текущий темп речи: '..v)
        return
      end
    elseif op == 'list' then
      if #t == 2 then
        local l = lvoice.getVoices()
        if type(l) == 'table' then
          print('[voice] Список голосов:')
          for id,v in ipairs(l) do
            print (id..':'..v)
          end
        else
          print('[voice] Ошибка получения списка голосов')
        end
        return
      end
    end
    return 'Неверный набор параметров'
  end
  return t
end
