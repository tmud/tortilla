-- lor
-- Плагин для Tortilla mud client

local lor = {}
local initialized = false
local lor_catch_mode = false
local lor_show_mode = false
local lor_trigger, lor_check, lor_tegs, lor_import_trigger, lor_import, lor_drop, lor_last
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
  'Поиск идет как по имени предмета, так и по тегам. Теги присваиваются автоматически,',
  'но можно добавлять и свои теги. Для этого нужно выполнить команды лортег <имя тега>.',
  'Команда лортег добавляет тег на последний предмет который вы смотрели командой лор.',
  'Теги, которые назначаются автоматически - настраиваются в конфигурационном файле плагина.',  
  'Возможен импорт в базу из текстовых файлов, подробности в справке '..p..'help lor.'  
  }
  return table.concat(s, '\r\n')
end
function lor.version()
  return '1.08'
end

local function print(s)
  _G.print('[lor]: '..s)
end

--#### Perpage output
local lor_output_perpage
local lor_output_count = 0
local lor_output_page = 0
local lor_output_pages = 0
local lor_output = {}

local function init_output()
  lor_output_count = 0
  lor_output_page = 0
  lor_output_pages = 0
  lor_output = {}
end

local function output(s)
  if not lor_output_perpage then
    _G.print(s)
    return
  end
  if lor_output_count < lor_output_perpage then
    lor_output_count = lor_output_count + 1
    _G.print(s)
  end
  lor_output[#lor_output+1] = s
end

local function end_output()
  local count = #lor_output
  if count > 0 then
    lor_output_pages = math.ceil( count / lor_output_perpage )
    lor_output_page = 1
    if lor_output_pages ~= 1 then
      print('Страница: '..lor_output_page..'/'..lor_output_pages)
    end
  end
end

local function next_ouput_page()
  local count = #lor_output
  if count == 0 then return false end
  lor_output_page = lor_output_page + 1
  if lor_output_page > lor_output_pages then lor_output_page = 1 end
  local from = (lor_output_page - 1) * lor_output_perpage + 1
  local to = from + lor_output_perpage - 1
  if to > count then to = count end
  for i=from,to do
    _G.print(lor_output[i])
  end
  if lor_output_pages ~= 1 then
    print('Страница: '..lor_output_page..'/'..lor_output_pages)
  end
  return true
end

local function first_output_page()
  lor_output_page = lor_output_pages
end
--##### Perpage output

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
    terminate("Ошибка в настройках, в параметре key.")
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
    if type(t.import_key) ~= 'string' then
      terminate("Ошибка в настройках. Для импорта необходим параметр import_key.")
    end
    lor_import_trigger = createPcre(t.import_key)
    if not lor_import_trigger then
       terminate("Ошибка в настройках, в параметре import_key.")
    end
  end
  lor_drop = nil
  if type(t.drop) == 'function' then
    lor_drop = t.drop()
    if type(lor_drop) ~= 'function' then
      terminate("Ошибка в настройках, в параметре drop должна быть возвращена функция.")
    end
  end
  if extra and type(extra.dictonary) == 'function' then
    local path = getPath("")
    lor_dictonary = extra.dictonary(path)
  end
  if not lor_dictonary then
    terminate("Не загружен модуль extra для работы с базой предметов.")
  end
  local pp = t.perpage
  if type(pp) == 'number' and pp > 10 then
    lor_output_perpage = pp
  else
    lor_output_perpage = 20
  end
  initialized = true
end

local function save_lor_strings()
  if not lor_strings.name then
    lor_strings = {}
    return false, "Не получено имя предмета, сохранить невозможно."
  end
  if lor_drop and not lor_drop(lor_strings.name) then
    lor_strings = {}
    return false
  end
  local info = {}
  local tegs = {}
  for k,s in ipairs(lor_strings) do
    if lor_tegs then
      local newtegs = lor_tegs(s:getText())
      if newtegs then
        if type(newtegs) == 'string' then
          tegs[#tegs+1] = newtegs
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
      lor_last = name
      return false, 'Предмет уже есть в базе: '..name
    end
    local errtext = err and ' Ошибка: '..err..'.' or ''
    return false, "Предмет не добавлен в базу из-за ошибки: "..name..errtext
  end
  lor_last = name
  return true, "Предмет добавлен в базу: "..name
end

local function print_object(name, info)
  lor_show_mode = true
  local t = info.data:tokenize('\r\n')
  local vs = createViewString()
  for i=1,#t do
    vs:setData(t[i])
    vs:print(0)
  end
  lor_show_mode = false
  lor_last = name
end

local function reteg(s)
  local tegs = {}
  local t = s:tokenize('\n')
  local vs = createViewString()
  for _,s in ipairs(t) do
    vs:setData(s)
    local newtegs = lor_tegs(vs:getText())
    if newtegs then
      if type(newtegs) == 'string' then
        tegs[#tegs+1] = newtegs
      elseif type(newtegs) == 'table' then
        for _,t in pairs(newtegs) do
          tegs[#tegs+1] = t
        end
      end
    end
  end
  return tegs
end

local function concat_tegs(info)
  local tegs1 = table.concat(info.auto, ',')
  local tegs2 = table.concat(info.tegs, ',')
  local tegs = ''
  if tegs1:len() > 0 then
    if tegs2:len() > 0 then
      tegs = tegs1..','..tegs2
    else
      tegs = tegs1
    end
  elseif tegs2:len() > 0 then
    tegs = tegs2
  end
  return tegs
end

local function find_lor_strings(id)
  if id:only('0123456789') then
    local index = tonumber(id)
    local t = lor_cache[index]
    if t then
      print_object(t.name, t.info)
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
    local name,info = next(t)
    print_object(name, info)
  else
    print("Уточните поиск (лор номер):")
    init_output()
    lor_cache = {}
    local c = lor_cache
    for name,info in pairs(t) do
      local id = #c+1
      c[id] = { name = name, info = info }
      local tegs = concat_tegs(info)
      if tegs:len()>0 then
        output(""..id..". "..name.." ("..tegs..")")
      else
        output(""..id..". "..name)
      end
    end
    end_output()
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
  lor_cache = {}
  local working = false
  for _,s in ipairs(t) do
    s = system.convertFromWin(s)
    local found_key = lor_import_trigger:find(s)
    if not working then
      if not found_key then goto next end
      working = true
    end
    if found_key then
      local _,result = save_lor_strings()
      if result then print(result) end
      lor_strings = {}
    end
    local vs = createViewString()
    vs:setBlocksCount(1)
    vs:setBlockText(1, s)
    local result, item = lor_import(vs)
    if item then lor_strings.name = item end
    if result then
      lor_strings[#lor_strings+1] = vs
    elseif result == nil then
      working = false  --drop
      lor_strings = {}
    end
    ::next::
  end
  if lor_strings.name then
    local _,result = save_lor_strings()
    if result then print(result) end
  end
end

local function lorteg(teg)
  if teg:len() == 0 then
    print("Ошибка: Укажите тег.")
    return
  end
  if not lor_last then
    print("Ошибка: тег присваивается или снимается с последнего осмотренного предмета.")
    return
  end
  local res = lor_dictonary:teg(lor_last, teg)
  if res == 'added' then
    lor_output = {}
    print("Добавлен тег '"..teg.."' для '"..lor_last.."'.")
    for _,t in ipairs(lor_cache) do
      if t.name == lor_last then
        local t = t.info.tegs
        t[#t+1] = teg
        break
      end
    end
  elseif res == 'removed' then
    lor_output = {}
    print("Тег '"..teg.."' удален для '"..lor_last.."'.")
    for _,t in ipairs(lor_cache) do
      if t.name == lor_last then
        local t = t.info.tegs
        for k,kteg in pairs(t) do
          if teg == kteg then table.remove(t, k) break end
        end
        break
      end
    end
  elseif res == 'absent' then
    print("Объекта '"..lor_last.."' нет в базе. Тег не добавлен.")
  elseif res == 'exist' then
    print("У объекта '"..lor_last.."' уже есть данный автотег.")
  else
    print("Ошибка: команда лортег не выполнилась.")
  end
end

function lor.gamecmd(t)
  if t[1] ~= "лор" and t[1] ~= "лортег" then return t end
  if not initialized then
    print("Ошибка в настройках плагина.")
    return
  end
  if t[1] == 'лортег' then
    local id = table.concat(t, '', 2)
    lorteg(id:trim())
    return
  end
  local id = ""
  for k=2,#t do
    if k ~= 2 then id = id..' ' end
    id = id..t[k]
  end
  id = id:trim()
  id = table.concat(id:tokenize('.'), ' ')
  if id == '0' then first_output_page() id="" end
  if id:len() == 0 then
    if #lor_cache > 0 then
      if lor_output_pages > 1 then
        print("Последний поиск (лор номер, 0-с первой страницы):")
      else
        print("Последний поиск (лор номер):")
      end
    end
    if next_ouput_page() then return end
    init_output()
    for id,t in ipairs(lor_cache) do
      local tegs = concat_tegs(t.info)
      if tegs:len() > 0 then
        output(""..id..". "..t.name.." ("..tegs..")")
      else
        output(""..id..". "..t.name)
      end
    end
    end_output()
    if #lor_cache == 0 then
      print("Что искать в базе?")
    end
  else
    find_lor_strings(id)
  end
end

local function print_help()
  local p = props.cmdPrefix()
  print(p..'lor import|reteg ('..p..'help lor).')
end

function lor.syscmd(t)
  if t[1] ~= 'lor' then return t end
  if not t[2] then
    print_help()
    return
  end
  if t[2] == 'import' then
    if not lor_import then
      print("Не заданы настройки для данной операции.")
      return
    end
    if not t[3] then
      print("Укажите имя файла")
    else
      import(t[3])
    end
    return
  end
  if t[2] == 'reteg' then
    if not lor_tegs then
      print("Не заданы настройки для данной операции.")
      return
    end
    print('Обновление тегов в базе...')
    lor_cache = {}
    local res, err = lor_dictonary:update(reteg)
    if res then
      print('Обновление тегов завершено.')
    else
      print('Обновление тегов не произошло. Ошибка: '..err)
    end
    return
  end
  print('Ошибка: Неизвестная команда.')
  print_help()
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
      local _,result = save_lor_strings()
      if result then
        vd:insertString(true, false)
        vd:select(i)
        vd:setBlocksCount(1)
        vd:setBlockText(1, '[lor] '..result)
      end
      break
    end
    if not vd:isSystem() and not vd:isGameCmd() and vd:getTextLen() > 0 then
      local vs = vd:createViewString()
      local result, item = lor_check(vs)
      if item then lor_strings.name = item end
      if result then
        lor_strings[#lor_strings+1] = vs
      elseif result == nil then  -- nil -> drop object
        lor_catch_mode = false
        break
      end
    end
  end
end

return lor
