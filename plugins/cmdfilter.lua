-- cmdfilter
-- Плагин для Tortilla mud client

-- список команд, которые нужно фильтровать
local cmd_list = { 'группа' }

local cmdfilter = {}
function cmdfilter.name() 
  return 'Фильтр команд'
end
function cmdfilter.description()
  local s = 'Плагин фильтрует заданные команды от попадания в окно клиента, остается только результат.'
  return s
end
function cmdfilter.version()
  return '-'
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
function cmdfilter.after(window, v)
if window ~= 0 then return end
  --[[for i=1,v:size() do --todo
    v:select(i)
    vprint(2,v:getText())
  end]]
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
          todelete[#todelete+1] = i
        end
      end
    end
    ::next::
  end
  for i=#todelete,1,-1 do
    v:select(todelete[i])
    v:deleteString()
  end
end

return cmdfilter
