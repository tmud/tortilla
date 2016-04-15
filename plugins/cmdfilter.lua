-- cmdfilter
-- Плагин для Tortilla mud client

-- список команд, которые нужно фильтровать
local cmd_list = { 'группа', 'счет', 'аф' }

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

local catch_mode = false
function cmdfilter.after(window, v)
if window ~= 0 then return end
  local todelete = {}
  for i=1,v:size() do
    v:select(i)
    if not catch_mode then
      if v:isGameCmd() then
        local last=v:blocks()
        local cmd = v:getBlockText(last)
        for _,c in ipairs(cmd_list) do
          if c == cmd then
            todelete[#todelete+1] = i
            catch_mode = true
          end
        end
      end
    else
      if v:isPrompt() then
        catch_mode = false
      end
      if v:getTextLen() == 0 then
        todelete[#todelete+1] = i
      end
    end
  end
  for i=#todelete,1,-1 do
    v:select(todelete[i])
    v:deleteString()
  end
end

return cmdfilter
