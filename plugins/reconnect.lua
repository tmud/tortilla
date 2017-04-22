-- reconnect
-- Плагин для Tortilla mud client

-- Количество попыток переподключения (0 - бесконечное количество)
local max_count = 10

-- Таймаут, при котором обрывается связь, если не поступают игровые данные от мад сервера
local timeout = 180

-- Переподключатся, если окно клиента активно в момент обрыва соединения (1 - да, 0 - нет)
local active_mode = 1

local reconnect = {}
function reconnect.name() 
    return 'Автореконнект'
end

function reconnect.description()
  local n = 'Количество попыток переподключения - бесконечно'
  if max_count and max_count > 0 then
    n = 'Количество попыток переподключения - '..max_count
  end
  local s = { 'Плагин переподключает клиент к серверу в случае обрыва связи.',
  'Настройки задаются прямо в файле плагина plugins/reconnect.lua',
  'Текущие настройки:', n, 
  'Автоматическое передподключение - через '..timeout..' секунд, если нет игровых данных.'
  }
  return table.concat(s, '\r\n')
end

function reconnect.version()
    return '1.07'
end

local connected = false
local address = nil
local port = nil
local attempts = 0
local timeout_seconds = 0
local timeout_disconnect = false

local function flash()
  if not props.activated() then
    flashWindow()
  end
end

function reconnect.syscmd(t)
  if #t == 1 and (t[1] == 'zap' or t[1] == 'disconnect') then
    if not timeout_disconnect then
      connected = false
    end
    timeout_disconnect = false
  end
  return t
end

function reconnect.connect()
  connected = true
  reconnect.getaddress()
  attempts = 0
  timeout_seconds = 0
end

function reconnect.disconnect()
  if not connected then
    return
  end
  if attempts == 0 then
    flash()
  end
  attempts = attempts + 1
  if max_count and max_count > 0 then
    if attempts == max_count+1 then
       connected = false
       flash()
       return
    end
  end
  if active_mode ~= 1 then
    if props.activated() then 
	  connected = false
	  timeout_disconnect = false
	  return 
	end
  end
  local p = props.cmdPrefix()
  local cmd = p..'output Переподключение('..attempts..')...'
  runCommand(cmd)
  if system and system.sleep then
    system.sleep(2000)
  end
  cmd = p..'connect '..address..' '..port
  runCommand(cmd)
end

function reconnect.init()
  connected = props.connected()
  reconnect.getaddress()
end

function reconnect.getaddress()
  if connected then
    address = props.serverHost()
    port = props.serverPort()
  else
    address = nil
    port = nil
  end
end

function reconnect.streamdata(stream)
  timeout_seconds = 0
  return stream
end

function reconnect.tick()
  timeout_seconds = timeout_seconds + 1
  if timeout_seconds > timeout then
    timeout_seconds = 0
    timeout_disconnect = true
    print('[автореконнект] Таймаут по отсутствию игровых данных. Переподключение...')
    local p = props.cmdPrefix()
    runCommand(p..'disconnect')
  end
end

return reconnect