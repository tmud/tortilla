-- lor
-- Плагин для Tortilla mud client
local lor = {}
local initialized = false
local lor_catch_mode = false
local lor_trigger, lor_filter, lor_import
local lor_strings = {}
local lor_dictonary

function lor.name()
  return 'База предметов'
end
function lor.description()
  local s = {
  'Плагин сохраняет в базе информацию о предметах, а также позволяет в этой базе искать их.',
  'Лор-информация собирается автоматически, когда предмет изучается в игре.',
  'Для поиска используется команда лор <имя предмета>, можно использовать сокращения.',
  'Возможен импорт в базу из текстовых файлов командой лоримпорт <имя файла>, но для',
  'этой возможности нужна своя настройка в конфигурационном файле плагина.',
  'Импорт работает с кодировкой win. См. справку по плагину: '..props.cmdPrefix()..'help lor'
  }
  return table.concat(s, '\r\n')
end
function lor.version()
  return '1.01'
end

local function print(s)
  _G.print('[lor]: '..s)
end

function lor.init()
  initialized = false
  local t = loadTable("config.lua")
  if not t or type(t.key) ~= 'string' or type(t.check) ~= 'function' then
    terminate("Ошибка в настройках.")
  end
  lor_trigger = createPcre(t.key)
  if not lor_trigger then
    terminate("Ошибка в настройках, в ключевой строке key.")
  end
  lor_filter = t.check(createPcre)
  if type(lor_filter) ~= 'function' then
    terminate("Ошибка в настройках, в параметре check должна быть возвращена функция.")
  end
  if type(t.import) == 'function' then
    lor_import = t.import(createPcre)
    if type(lor_import) ~= 'function' then
      terminate("Ошибка в настройках, в параметре import должна быть возвращена функция.")
    end
  end
  if extra and type(extra.dictonary) == 'function' then
    lor_dictonary = extra.dictonary()
  end
  if not lor_dictonary then
    terminate("Не загружен модуль extra для работы с базой предметов.")
  end
  initialized = true
end

local function save_lor_strings()
  if not lor_strings.name then
    return false, "Не получено имя предмета, сохранить невозможно."
  end
  local info = {}
  for k,s in ipairs(lor_strings) do
    info[k] = s:getData()
  end
  local res,err = lor_dictonary:add(lor_strings.name, table.concat(info,'\n'))
  if not res then
    if err == 'exist' then
      return false, "Предмет уже существует в базе."
    end
    return false, "Предмет не добавлен в базу из-за ошибки."
  end
  return true, "Предмет добавлен в базу."
end

local function find_lor_strings(id)
  local t = lor_dictonary:find(id)
  if not t then
    print("Ничего не найдено")
    return
  end
  local info = t:tokenize('\n')
  local vs = createViewString()
  for _,s in ipairs(info) do
    vs:setData(s)
    vs:print(0)
  end
end

local function import(file)
  if not system then
    print('Ошибка! Не загружен модуль system. Импорт невозможен.')
    return
  end
  local t = system.loadTextFile(file)
  if not t then
    print('Ошибка! Файл '..file..' загрузить не получилось.')
    return
  end
  for _,s in ipairs(t) do
    s = system.convertFromWin(s)
    local vs = createViewString()
    local result, item = lor_import(s, vs)
    if result then
       if item then
         save_lor_strings()
         lor_strings = {}
       end
      lor_strings[#lor_strings+1] = vs
      if item then lor_strings.name = item end
    end
  end
end

function lor.gamecmd(t)
  if t[1] ~= "лор" and t[1] ~= 'лоримпорт' then
    return t
  end
  if not initialized then
    print("Ошибка в настройках.")
    return nil
  end
  if t[1] == 'лоримпорт' then
    if not lor_import then
      print("Ошибка в настройках.")
      return nil
    end
    if not t[2] then
      print("Укажите имя файла")
    else
      import(t[2])
    end
    return {}
  end
  local id = ""
  for k=2,#t do
    if k ~= 2 then id = id..' ' end
    id = id..t[k]
  end
  if id:len() == 0 then
    print("Что искать в базе?")
  else
    find_lor_strings(id)
  end
  return {}
end

function lor.before(v, vd)
  if v ~= 0 then return end
  if not lor_trigger then
    terminate("Ошибка в настройках.")
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
      local res,error = save_lor_strings()
      if error then
        vd:insertString(true, false)
        vd:select(i)
        vd:setBlocksCount(1)
        vd:setBlockText(1, '[lor] '..error)
      end
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
