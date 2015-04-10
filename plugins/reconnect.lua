-- reconnect
-- Плагин для Tortilla mud client

reconnect = {}
function reconnect.name() 
    return 'Автореконнект'
end

function reconnect.description()
return 'Плагин переподключает клиент к серверу в случае обрыва связи.'
end

function reconnect.version()
    return '-'
end

local connected = false
local address = nil
local port = nil

function reconnect.syscmd(t)
  if #t == 1 and (t[1] == 'zap' or t[1] == 'disconnect') then
    connected = false
  end
  return t
end

function reconnect.connect()
  connected = true
  reconnect.getaddress()
end

function reconnect.disconnect()
  if connected then
    local p = props.cmdPrefix()
    local cmd = p..'output Переподключение...'
    runCommand(cmd)
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