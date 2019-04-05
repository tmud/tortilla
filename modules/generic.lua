-- generic.lua
-- модуль для создания общих триггеров для всех персонажей

if not checkVersion(1,13) then
  print ("[generic] Модуль generic не загружен. Требуется версия клиента 1.13+")
  return
end

gentr = {}

local function str_ne(s)
  if type(s) ~= "string" then return false end
  if string.len(s) == 0 then return false end
  return true
end

local function str_nil(s)
  if s == nil then return true end
  if type(s) == "string" then return true end
  return false
end

local function err(...)
  local e = ""
  for i,v in ipairs(arg) do
    e = e .. tostring(v)
  end
  print("[generic] Ошибка: "..e)
end

local function doaction(vd, command)
  command = vd:translate(command)
  runCommand(command)
end

local function dosub(vd, substring, color)
  if color == nil then color = "" end
  if substring == nil then
    vd:replace(color)
  else
    vd:replace(color, substring)
  end
end

function gentr.action(macro, command)
  if str_ne(macro) and str_ne(command) then
    createTrigger(macro, function(vd) doaction(vd, command) end)
    return
  end
  err("action(", macro, ",", command)
end

function gentr.sub(macro, substring, color)
  if str_ne(macro) and str_nil(substring) and str_nil(color) then
    local scolor = ""
    if color then  scolor = createColor(color) end
    if scolor then
      createTrigger(macro, function(vd) dosub(vd, substring, scolor) end)
      return
    end
  end
  err("sub(", macro, ",", substring, ",", color, ")")
end

function gentr.gag(macro)
  if str_ne(macro) then
    createTrigger(macro, function(vd) dosub(vd, "") end)
    return
  end
  err("gag(", macro,  ")")
end

function gentr.highlight(macro, color)
  if str_ne(macro) and str_ne(color) then
    local scolor = createColor(color)
    if scolor then
      createTrigger(macro, function(vd) dosub(vd, nil, scolor) end)
      return
    end
  end
  err("highlight(", macro, ",", color, ")")
end
