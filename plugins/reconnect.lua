-- reconnect
-- Плагин для Tortilla mud client

-- Количество попыток переподключения (0 - бесконечное количество)
local max_count = 10

local reconnect = {}
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
    return '1.05'
end

local connected = false
local address = nil
local port = nil
local attempts = 0
local function flash()
  if not props.activated() then
    flashWindow()
  end
end

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

return reconnect