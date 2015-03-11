-- reconnect
-- Плагин для Tortilla mud client

reconnect = {}
function reconnect.name() 
    return 'Автореконнект'
end

function reconnect.description()
return 'Плагин переподключает клиент в случае обрыва связи с сервером'
end

function reconnect.version()
    return '-'
end

local connected = false
local address = nil
local port = nil

function reconnect.syscmd(t)
  if t[1] == 'connect' and t[2] and t[3] then
    address = t[2]
    port = t[3]
    log('address:'..address..', port:'..port)
  end
  if #t == 1 and (t[1] == 'zap' or t[1] == 'disconnect') then
    connected = false
  end
  return cmd
end

function reconnect.connect()
  connected = true
  log('connect !')
end

function reconnect.disconnect()
  if connected and address and port then 
    log('reconnect !')
    log('address:'..address..', port:'..port)
  else
    address = nil
    port = nil
  end
end

