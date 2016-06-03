-- lor
-- Плагин для Tortilla mud client

local lor = {}
local initialized = false
local lor_catch_mode = false
local lor_show_mode = false
local lor_trigger, lor_check, lor_tegs, lor_import
local lor_strings = {}
local lor_dictonary
local lor_cache = {}

function lor.name()
  return 'База предметов'
end
function lor.description()
  local p = props.cmdPrefix()
  local s = {
  'Плагин сохраняет в базе информацию о предметах, а также позволяет в этой базе искать их.',
  'Лор-информация собирается автоматически, когда предмет изучается в игре.',
  'Для поиска используется команда лор <имя предмета>, можно использовать сокращения.',
  'Возможен импорт в базу из текстовых файлов командой '..p..'lorimport <имя файла>, но для',
  'этой возможности нужна своя настройка в конфигурационном файле плагина. Импорт работает с ',
  'кодировкой win. Также можно искать предметы по тегам. Для пересчета тегов всей базы',
  'используется команда '..p..'lorupdate. Выполнение команды можент занять заметное время.',
  'Для поддержки тегов нужна своя настройка в конфигурационном файле плагина.',
  'См. справку по плагину: '..p..'help lor.'
  }
  return table.concat(s, '\r\n')
end
function lor.version()
  return '1.04'
end

local function output(s)
  _G.print(s)
end
local function print(s)
  _G.print('[lor]: '..s)
end

function lor.init()
  initialized = false
  local t = loadTable("config.lua")
  if t and type(t.init) == 'function' then
    t.init()
  end
  if not t or type(t.key) ~= 'string' or type(t.check) ~= 'function' then
    terminate("Ошибка в настройках. См. справку.")
  end
  lor_trigger = createPcre(t.key)
  if not lor_trigger then
    terminate("Ошибка в настройках, в ключевой строке key.")
  end
  lor_check = t.check()
  if type(lor_check) ~= 'function' then
    terminate("Ошибка в настройках, в параметре check должна быть возвращена функция.")
  end
  lor_tegs = nil
  if type(t.tegs) == 'function' then
    lor_tegs = t.tegs()
    if type(lor_tegs) ~= 'function' then
      terminate("Ошибка в настройках, в параметре tegs должна быть возвращена функция.")
    end
  end
  lor_import = nil
  if type(t.import) == 'function' then
    lor_import = t.import()
    if type(lor_import) ~= 'function' then
      terminate("Ошибка в настройках, в параметре import должна быть возвращена функция.")
    end
  end
  if extra and type(extra.dictonary) == 'function' then
    local path = getPath("")
    lor_dictonary = extra.dictonary(path)
  end
  if not lor_dictonary then
    terminate("Не загружен модуль extra для работы с базой предметов.")
  end
  initialized = true
end

local function save_lor_strings()
  if not lor_strings.name then
    lor_strings = {}
    return false, "Не получено имя предмета, сохранить невозможно."
  end
  local info = {}
  local tegs = {}
  for k,s in ipairs(lor_strings) do
    if lor_tegs then
      local newtegs = lor_tegs(s:getText())
      if newtegs then
        if type(newtegs) == 'string' then
          tegs[#tegs+1] = teg
        elseif type(newtegs) == 'table' then
          for _,t in pairs(newtegs) do
            tegs[#tegs+1] = t
          end
        else
          return false, "Ошибка при вызове функции tegs"
        end
      end
    end
    info[k] = s:getData()
  end
  local res,err = lor_dictonary:add(lor_strings.name, table.concat(info,'\n'), tegs)
  local name = lor_strings.name..'.'
  lor_strings = {}
  if not res then
    if err == 'exist' then
      return false, 'Предмет уже есть в базе: '..name
    end
    local errtext = err and ' Ошибка: '..err..'.' or ''
    return false, "Предмет не добавлен в базу из-за ошибки: "..name..errtext
  end
  return true, "Предмет добавлен в базу: "..name
end

local function print_object(s)
  lor_show_mode = true
  local t = s:tokenize('\n')
  local vs = createViewString()
  for _,s in ipairs(t) do
    vs:setData(s)
    vs:print(0)
  end
  lor_show_mode = false
end

local function find_lor_strings(id)
  if id:only('0123456789') then
    local index = tonumber(id)
    local t = lor_cache[index]
    if t then
      print_object(t.data)
    else
      print("Ничего не найдено")
    end
    return
  end
  local t = lor_dictonary:find(id)
  if not t then
    print("Ничего не найдено")
    return
  end
  local count = 0
  for _ in pairs(t) do count = count + 1 end
  if count == 1 then
    local _,info = next(t)
    print_object(info)
  else
    print("Уточните поиск (лор номер):")
    lor_cache = {}
    local c = lor_cache
    for name,data in pairs(t) do
      local id = #c+1
      c[id] = { name = name, data = data }
      output(""..id..". "..name)
    end
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
    if s:len() > 0 then
      local vs = createViewString()
      local result, item = lor_import(s, vs)
      if result then
        if item then
          if lor_strings.name then
            local _,result = save_lor_strings()
            if result then print(result) end
          end
          lor_strings.name = item
        end
        lor_strings[#lor_strings+1] = vs
        if item then lor_strings.name = item end
      end
    end
  end
  if lor_strings.name then
    local _,result = save_lor_strings()
    if result then print(result) end
  end
end

function lor.gamecmd(t)
  if t[1] ~= "лор" then return t end
  if not initialized then
    print("Ошибка в настройках.")
    return nil
  end
  local id = ""
  for k=2,#t do
    if k ~= 2 then id = id..' ' end
    id = id..t[k]
  end
  id = table.concat(id:tokenize('.'), ' ')
  if id:len() == 0 then
    if #lor_cache > 0 then
      print("Последний поиск:")
    end
    for id,t in ipairs(lor_cache) do
      output(""..id..". "..t.name)
    end
    print("Что искать в базе?")
  else
    find_lor_strings(id)
  end
  return {}
end

function lor.syscmd(t)
  if t[1] == 'lorimport' then
    if not lor_import then
      print("Не заданы настройки для данной операции.")
      return nil
    end
    if not t[2] then
      print("Укажите имя файла")
    else
      import(t[2])
    end
    return {}
  end
  if t[1] == 'lorupdate' then
    if not lor_tegs then
      print("Не заданы настройки для данной операции.")
      return nil
    end
    lor_dictonary:update(lor_tegs)
    return {}
  end
  return t
end

function lor.before(v, vd)
  if v ~= 0 then return end
  if not lor_trigger then
    terminate("Ошибка в настройках.")
  end
  if not lor_catch_mode and not lor_show_mode then
    if vd:find(lor_trigger) then
      lor_catch_mode = true
      local index,size = vd:getIndex(),vd:size()
      if index == size then return end
      vd:select(index)
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
      break
    end
    if not vd:isSystem() and not vd:isGameCmd() and vd:getTextLen() > 0 then
      local ref = vd:createRef()
      local item
      ref, item = lor_check(ref)
      if ref then
        lor_strings[#lor_strings+1] = ref
        if item then lor_strings.name = item end
      elseif ref == false then  -- false -> drop object
        lor_catch_mode = false
        break
      end
    end
  end
end

return lor
