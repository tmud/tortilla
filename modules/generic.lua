-- generic.lua
-- модуль для создания общих триггеров для всех персонажей

gentr = {}

local function check_string_not_empty(s)
  if type(s) ~= "string" then return false end
  if string.len(s) == 0 then return false end
  return true
end

local function check_string(s)
  if type(s) ~= "string" then return false end
  return true
end

local function check_not_empty(what, trigger, p)
  if not check_string_not_empty(trigger) or not check_string_not_empty(p) then
    local msg = "[generic] Ошибка: "..what.."("..tostring(trigger)..", "..tostring(p)..")"
    print(msg)
    return false
  end
  return true
end

local function check(what, trigger, p)
  if not check_string_not_empty(trigger) or not check_string(p) then
    local msg = "[generic] Ошибка: "..tostring(what).."("..tostring(trigger)..", "..p..")"
    print(msg)
    return false
  end
  return true
end

local function doaction(vd, command)
  command = vd:translate(command)
  runCommand(command)
end

local function dosub(vd, substring)
  vd:replace(substring)
end

function gentr.action(macro, command)
  if not check_not_empty("act", macro, command) then return end
  createTrigger(macro, function(vd) doaction(vd, command) end)
end

function gentr.sub(macro, substring)
  if not check("sub", macro, substring) then return end
  createTrigger(macro, function(vd) dosub(vd, substring) end)
end

function gentr.gag(macro)
  if not check("gag", macro, "") then return end
  createTrigger(macro, function(vd) dosub(vd, "") end)
end

function gentr.highlight(macro, color)
end

