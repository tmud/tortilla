-- cmdfilter
-- Плагин для Tortilla mud client

-- список команд, которые нужно фильтровать
local cmd_list = {}

local cmdfilter = {}
function cmdfilter.name() 
  return 'Фильтр команд'
end

function cmdfilter.description()
  local s = 'Плагин фильтрует заданные команды от попадания в окно клиента.\r\nУдобно использовать, если игровые команды используются в таймерах.\r\n'
  s = s.."Список команд находится в файле: "..getPath('config.lua')
  return s
end

function cmdfilter.version()
  return '1.0'
end

function cmdfilter.init()
  local t = loadTable('config.lua')
  if not t or not t.cmdlist or type(t.cmdlist) ~= 'table' then
    terminate('Ошибка в настройках '..getPath('config.lua')..', нет списка команд cmdlist.')
  end
  cmd_list = t.cmdlist
end

function cmdfilter.after(window, v)
if window ~= 0 then return end
  local todelete = {}
  for i=1,v:size() do
    v:select(i)
    if v:isGameCmd() then
      local last=v:blocks()
    local cmd = v:getBlockText(last)
      for _,c in ipairs(cmd_list) do
         if c == cmd then todelete[#todelete+1] = i; break; end
      end
    end
  end
  for i=#todelete,1,-1 do
    v:select(todelete[i])
    v:deleteString()
  end
end

return cmdfilter
