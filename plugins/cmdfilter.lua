-- cmdfilter
-- Плагин для Tortilla mud client

local cmd_list = {}

local cmdfilter = {}
function cmdfilter.name() 
  return 'Фильтр команд'
end
function cmdfilter.description()
  local s = 'Плагин фильтрует заданные команды от попадания в окно клиента, остается только результат.\r\nУдобно использовать, если игровые команды используются в таймерах.\r\n'
  s = s.."Список команд находится в файле: "..getPath('config.lua')
  return s
end
function cmdfilter.version()
  return '-'
end

function cmdfilter.init()
  local t = loadTable('config.lua')
  if not t or not t.cmdlist or type(t.cmdlist) ~= 'table' then
    terminate('Ошибка в настройках '..getPath('config.lua')..', нет списка команд cmdlist.')
  end
  cmd_list = t.cmdlist
end

local function check_cmd(v)
  if not v:isGameCmd() then return end
  local last=v:blocks()
  local cmd = v:getBlockText(last)
  for _,c in ipairs(cmd_list) do
    if c == cmd then
      return true
    end
  end
  return false
end

local catch_mode = false
local prompts_counter = 0

function cmdfilter.disconnect()
  catch_mode = false
  prompts_counter = 0
end

function cmdfilter.after(window, v)
if window ~= 0 then return end
  local todelete = {}
  for i=1,v:size() do
    v:select(i)
    if not catch_mode then
      if check_cmd(v) then
        todelete[#todelete+1] = i
        catch_mode = true
      end
    else
      if check_cmd(v) then
        todelete[#todelete+1] = i
        prompts_counter = prompts_counter + 1
        goto next
      end
      if v:getTextLen() == 0 then
        todelete[#todelete+1] = i
        goto next
      end
      if v:isPrompt() then
        if prompts_counter == 0 then
          catch_mode = false
        else
          prompts_counter = prompts_counter - 1
        end
      end
    end
    ::next::
  end
  v:deleteStrings(todelete)
end

return cmdfilter
