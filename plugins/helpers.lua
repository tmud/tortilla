-- helpers
-- Плагин для Tortilla mud client

local commands = {
-- Чтобы отключить ненужную команду, нужно добавить -- в начале строки
  alert = 'Немодальное окно с текстом из параметров.',
  wactive = 'Выполняет команды из параметра, если окно клиента активно.',
  wnotactive = 'Выполняет команды из параметра, если окно клиента неактивно.',
  gmaskon = 'Включает группы триггеров по регулярке.',
  gmaskoff = 'Выключает группы триггеров по регулярке.',
  gmaskshow = 'Показывает группы триггеров подпадающих под регулярку.',
}

local clist
local function makelist()
  clist = {}
  for k in pairs(commands) do clist[k] = true end
end

local impl = {}
function impl.alert(p)
  system.alert(p)
  focusWindow()
end

local function runcmds(t)
  local cmd = ''
  local s = props.cmdSeparator()
  for k,v in ipairs(t) do
    if k~=1 then
      cmd = cmd..s
    end
    cmd = cmd..v
  end
  runCommand(cmd)
end

function impl.wactive(p)
  if props.activated() then
    runcmds(p)
  end
end

function impl.wnotactive(p)
  if not props.activated() then
    runcmds(p)
  end
end

local function select_groups(t)
  local glist = {}
  for _,v in ipairs(t) do
    local r = createPcre(v)
    for i=1,groups:size() do
      groups:select(i)
      local name = groups:get('key')
      if r:find(name) then
        glist[#glist+1] = i
      end
    end
  end
  return glist
end

local function group(t, mode)
  local glist = select_groups(t)
  if #glist > 0 then
    local gname = {}
    local m = mode and '1' or '0'
    local action = mode and 'включены' or 'отключены'
    for k,v in ipairs(glist) do
      groups:select(v)
      gname[k] = groups:get('key')
      groups:set('value', m)
    end
    print('[gmask] Группы '..action..': '..table.concat(gname, ','))
    groups:update()
  end
end

function impl.gmaskon(p)
  group(p, true)
end

function impl.gmaskoff(p)
  group(p, false)
end

function impl.gmaskshow(p)
  local glist = select_groups(p)
  if #glist == 0 then
    print('[gmask] Нет групп, соотвествующих регулярке.')
    return
  end
  print('[gmask] Группы, которые соотвествуют регулярке:')
  for k,v in ipairs(glist) do
    groups:select(v)
    print(groups:get('key'))
  end
end

local helpers = {}
function helpers.name()
  return 'Сборник различных команд'
end
function helpers.description()
  local s = { 'Плагин добавляет несколько вспомогательных команд в клиент.',
  'Ненужные команды можно отключить в файле плагина.', 'Список команд:' }
  local p = props.cmdPrefix()
  for k,v in pairsByKeys(commands) do
    s[#s+1] = p..k..' - '..v
  end
  return table.concat(s, '\r\n')
end

function helpers.version()
  return '1.01'
end
function helpers.init()
  makelist()
end
function helpers.syscmd(t)
  if not clist[t[1]] then return t end
  local f = impl[t[1]]
  if not f then return t end
  local p = {}
  for i=2,#t do p[i-1] = t[i] end
  f(p)
end

return helpers