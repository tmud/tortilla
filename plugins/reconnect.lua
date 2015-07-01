-- reconnect
-- Плагин для Tortilla mud client

-- количество попыток переподключения (0 - бесконечное количество)
local max_count = 10

reconnect = {}
function reconnect.name() 
    return 'Автореконнект'
end

function reconnect.description()
  local s = 'Плагин переподключает клиент к серверу в случае обрыва связи.\r\nНастройки задаются прямо в файле плагина plugins/reconnect.lua\r\nТекущие настройки:\r\n'
  if max_count and max_count > 0 then
    s = s..'Количество попыток переподключения - '..max_count
  else
    s = s..'Количество попыток переподключения - бесконечно'
  end
  return s
end

function reconnect.version()
    return '1.01'
end

local connected = false
local address = nil
local port = nil
local attempts = 0

function reconnect.syscmd(t)
  if #t == 1 and (t[1] == 'zap' or t[1] == 'disconnect') then
    connected = false
  end
  return t
end

function reconnect.connect()
  connected = true
  reconnect.getaddress()
  attempts = 0
end

function reconnect.disconnect()
  if connected then
    if max_count and max_count ~= 0 then
        if attempts == max_count then
            connected = false
            flashParent()
            return
        end
        attempts = attempts + 1
    end
    local p = props.cmdPrefix()
    local cmd = p..'output Переподключение...'
    runCommand(cmd)
    if system and system.sleep then
        system.sleep(1000)
    end
    cmd = p..'connect '..address..' '..port
    runCommand(cmd)
  end
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