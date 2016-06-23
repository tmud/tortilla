-- cmdfilter
-- Плагин для Tortilla mud client

local cmd_list = {}
local scmd_list = {}

local cmdfilter = {}
function cmdfilter.name() 
  return 'Фильтр команд'
end
function cmdfilter.description()
  local s = { 'Плагин фильтрует заданные команды от попадания в окно клиента, остается только результат.',
  'Удобно использовать, если игровые команды используются в таймерах или триггерах.',
  "Список команд находится в файле: "..getPath('config.lua') }
  return table.concat(s, '\r\n')
end
function cmdfilter.version()
  return '1.03'
end

function cmdfilter.init()
  local t = loadTable('config.lua')
  if not t then
    terminate('Ошибка в файле настроек '..getPath('config.lua')..', нет списка команд.')
  end
  if (not t.cmdlist or type(t.cmdlist) ~= 'table') and (not t.scmdlist or type(t.scmdlist) ~= 'table') then
    terminate('Ошибка в настройках '..getPath('config.lua')..', нет списка команд cmdlist или scmdlist.')
  end
  cmd_list = {} 
  scmd_list = {}
  if t.cmdlist then
    for _,cmd in pairs(t.cmdlist) do
      cmd_list[cmd] = true
    end
  end
  if t.scmdlist then
    for _,cmd in pairs(t.scmdlist) do
      scmd_list[cmd] = true
    end
  end
end

local function check_cmd(v)
  if not v:isGameCmd() then return end
  local last=v:blocks()
  local cmd = v:getBlockText(last)
  return cmd_list[cmd] and true or false
end

local function check_scmd(v)
  if not v:isGameCmd() then return end
  if v:blocks() == 0 then return end
  local cmd = v:getBlockText(v:blocks())
  local from = 1
  if cmd:substr(1,1) == '-' then from = 2 end
  if cmd:substr(from,1) ~= props.cmdPrefix() then return end
  local c = cmd:substr(from+1, cmd:len()-from)
  c = c:tokenize(' ')
  return scmd_list[ c[1] ] and true or false
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
  local isnext = false
  for i=1,v:size() do
    v:select(i)
    if isnext then
      todelete[#todelete+1] = i
      goto next
    end
    if check_scmd(v) then
      todelete[#todelete+1] = i
      goto next
    end
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
    isnext = false
    local last = #todelete
    if v:isNext() and last > 0 and todelete[last] == i then
      isnext = true
    end
  end
  v:deleteStrings(todelete)
end

return cmdfilter
