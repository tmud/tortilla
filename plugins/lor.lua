-- lor
-- Плагин для Tortilla mud client
local lor = {}
local initialized = false
local lor_catch_mode = false
local lor_trigger, lor_filter
local lor_strings = {}

function lor.name()
  return 'База предметов'
end
function lor.description()
  return 'Плагин сохраняет в базе информацию о предметах, а также позволяет в этой базе искать их.'
end
function lor.version()
  return '1.0'
end

function lor.init()
  initialized = false
  local t = loadTable("config.lua")
  if not t or type(t.key) ~= 'string' or type(t.check) ~= 'function' then
    terminate("Ошибка в настройках.")
    return
  end
  lor_trigger = createPcre(t.key)
  if not lor_trigger then
    terminate("Ошибка в ключевой строке key, в настройках.")
    return
  end
  lor_filter = t.check(createPcre)
  if type(lor_filter) ~= 'function' then
    terminate("Ошибка в параметре check, должна быть возвращена функция.")
    return
  end
  initialized = true
end

local function save_lor_strings()
  for _,s in ipairs(lor_strings) do
    s:print(1)
  end
end

local function find_lor_strings(id)
print("ЛОР:"..id)
end

function lor.gamecmd(t)
  if t[1] == "лор" then
    if not initialized then
      print("[лор]: Ошибка в настройках.")
      return
    end
    local id = ""
    for k=2,#t do
      if k ~= 2 then id = id..' ' end
      id = id..t[k]
    end
    find_lor_strings(id)
    return
  end
  return t
end

function lor.before(v, vd)
  if v ~= 0 then return end
  if not lor_trigger then
    terminate("Ошибка в настройках.")
    return
  end
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
      save_lor_strings()
      lor_strings = {}
      break
    end
    if not vd:isSystem() and not vd:isGameCmd() and vd:getTextLen() > 0 then 
      local ref = vd:createRef()
      local item
      ref, item = lor_filter(ref)
      if ref then
        lor_strings[#lor_strings+1] = ref
        if item then lor_strings.name = item end
      end
    end
  end
end

return lor
