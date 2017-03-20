-- helpers
-- Плагин для Tortilla mud client

local commands = {
-- Чтобы отключить ненужную команду, нужно добавить -- в начале строки
  alert = 'Немодальное окно с текстом из параметров.',
  wactive = 'Выполняет команды из параметра, если окно клиента активно.',
  wnotactive = 'Выполняет команды из параметра, если окно клиента неактивно.',
}

local clist
local function makelist()
  clist = {}
  for k,_ in pairs(commands) do clist[k] = true end
end

local impl = {
alert = function(...)
  local m = table.concat(..., ' ')
  system.msgbox(m)
end,
wactive = function(...)
  if props.activated() then
  end
end,
wnotactive = function(...)
  if not props.activated() then
  end
end
}

local helpers = {}
function helpers.name()
  return 'Сборник различных команд'
end
function helpers.description()
  makelist()
  local s = { 'Плагин добавляет несколько вспомогательных команд в клиент.',
  'Ненужные команды можно отключить в файле плагина.', 'Список команд:' }
  local p = props.cmdPrefix()
  for k,v in pairsByKeys(commands) do
    s[#s+1] = p..k..' - '..v
  end
  return table.concat(s, '\r\n')
end
function helpers.version()
  return '1.0'
end

function helpers.init()
  makelist()
end

function helpers.syscmd(t)
  if not clist[t[1]] then return t end
  local f = impl[t[1]]
  if not f then return t end
  local p = {}
  for i=2,#t do p[#p+1] = t[i] end
  f(p)
end

return helpers