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

function reconnect.barcmd(t)
  for _,v in ipairs(t) do
    log(v)
  end
  return t
end

function reconnect.syscmd(cmd)
  log(cmd)
  return cmd
end

function reconnect.disconnect()
end

function reconnect.init()
end
