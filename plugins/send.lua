-- send
-- Плагин для Tortilla mud client

send = {}
function send.name()
  return 'Команды в другие Tortilla'
end

function send.version()
  return '1.0'
end

function send.description()
  local p=props.cmdPrefix()
  local s = 'Плагин управления несколькими клиентами Tortilla из любого его экземпляра.\r\n'
  s = s..'Внимание! Игра в несколько окон нарушает правила мультинга на некоторых мад-серверах.\r\n'
  s = s..'Если на сервере запрещен мультинг, то вы рискуете быть заблокированным.\r\n'
  s = s..'Плагин добавляет команды '..p..'send и '..p..'sendall, которые отправляют команды в другие клиенты.\r\n'
  s = s..'Формат команд: '..p..'send {окно} {команда} - отправляет команду в нужный экземмпляр клиента.\r\n'
  s = s..'В качестве имени окна нужно использовать имя игрового профиля (в заголовке окна).\r\n'
  s = s..p..'sendall {команда} - отправка сразу во все окна клиента, в том числе текущий.\r\n'
  s = s..'Клиенту не требуется данный плагин, чтобы получать команды, он нужен\r\nтолько для их отправки.'
  return s
end

function send.init()
  addCommand('send')
  addCommand('sendall')
end

function send.syscmd(t)
  local c = t[1]
  if c == 'send' or c == 'sendall' then
    local params = {}
    if c == 'send' then
    for i=3,#t do params[i-2] = t[i] end
      sendCommand(t[2], table.concat(params, ' '))
    else
      for i=2,#t do params[i-1] = t[i] end
      sendCommand(nil, table.concat(params, ' '))
    end
    return nil
  end
  return t
end
