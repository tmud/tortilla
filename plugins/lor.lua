-- lor
-- Плагин для Tortilla mud client

local lor = {}
local initialized = false
local lor_catch_mode = false
local lor_trigger

function lor.name()
  return 'База предметов'
end

function lor.description()
  return 'Плагин сохраняет в базе информацию о предметах, а также позволяет искать в этой базе.'
end

function lor.version()
  return '1.0'
end

function lor.init()
  initialized = false
  local t = loadTable("config.lua")
  if not t or type(t.key) ~= 'string' or type(t.check) ~= 'function' then
    log("Ошибка в настройках.")
    return
  end
  lor_trigger = createPcre(t.key)
  if lor_trigger then

  end
end

function lor.gamecmd(t)
  if t[1] == "лор" then
    if not initialized then
      log("Ошибка в настройках.")
      return
    end
    local object = ""
    for k=2,#t do
      if k ~= 2 then object = object..' ' end
      object = object..t[k]
    end
    print("ЛОР:"..object)
    return
  end
  return t
end

function lor.before(v, vd)
  if v ~= 0 then return end
  if not lor_catch_mode then
    if vd:find(lor_trigger) then
      lor_catch_mode = true
      local index,size = vd:getIndex(),vd:size()
      if index == size then return end
      vd:select(index+1)
    end
  end
  if not lor_catch_mode then return end
  local index,size = vd:getIndex(), vd:size()
  for i=index,size do
    vd:select(i)
    if vd:isPrompt() then
      lor_catch_mode = false
      break
    end
    if not vd:isSystem() and not vd:isGameCmd() then 
      local item = vd:getText()
      if item ~= "" then
        --[[local object, param = t.func(item)
        local number = tonumber(param)
        if number then param = number end
        if t.ip then db.add(object) end
        t.main(object, param)]]
      end
    end
  end
end

return lor
