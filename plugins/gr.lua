local gr = {}
function gr.name()
  return 'Обработка группы в отдельном окне'
end
function gr.description()
  return ''
end
function gr.version()
  return '1.2'
end
local anchor = createPcre("\\[.*\\].*")
-- ОПРЕДЕЛЕНИЕ ПЕРЕМЕННЫХ
-- СЕКЦИЯ ПЕРЕМЕННЫХ ПЛАГИНА ТАГГИНГА ГРУППЫ
local tag, grouptable, flagassignation = createPcre("^([А-Я][а-я]*)( *)\\|"), {}, false
local groupflag = createPcre("Ваша группа состоит из\\:")
local groupDisband = createPcre("Вы распустили группу\\.")
local groupCreate = createPcre("Вы приняты в группу.*")
local notingroup = createPcre("^Но вы же не член \\(в лучшем смысле этого слова\\) группы\\!")
-- СЕКЦИЯ ПЕРЕМЕННЫХ ПЛАГИНА ОПРЕДЕЛЕНИЯ ПАДЕЖЕЙ ИМЕН СОБСТВЕННЫХ
local sogl = createPcre("[бвгджзклмнпрстфхцчшщ]")
local okonM, okonW = {"","а","у","а","ом","е"}, {"а","ы","е","у","ой","е"}
local okonM2, okonW2 = {"й","я","ю","я","ем","е"}, {"я","и","и","ю","ей","и"}
local okonM3 = {"ь","я","ю","я","ем","е"}
local result = {}
local padeg = {"и","р","д","в","т","п"}


local function xz(word, str, num) --функция определения падежа, корректировки окончания ИМЯ, ОКОНЧАНИЕ, НОМЕРВТАБЛИЦЕПАДЕЖЕЙ
  if sogl:find(str) then return word..okonM[num]
    elseif str == ("а") then return word:substr(1, word:len()-1)..okonW[num]
    elseif str == ("й") then return word:substr(1, word:len()-1)..okonM2[num]
    elseif str == ("я") then return word:substr(1, word:len()-1)..okonW2[num]
    elseif str == ("ь") then return word:substr(1, word:len()-1)..okonM3[num]
  end
end

local function grinresult(word, padName) --функция нумерации таблицы падежей, при совпадении искомого падежа пуш функции xz ИМЯ, ПАДЕЖ
  for _,v in pairs(padeg) do
    if v == padName then
      return xz(word, word:substr(word:len(),1), _)
    end
  end
end

local function assignvars() --функция пуша переменных в клиент вида gr<номер в группе> (именительный падеж), gr<новер в группе>r (родительный падеж)
	for i = 1, #grouptable do
		vars:select("gr"..i)
			if  vars:get("value") ~= grouptable[i].name then
				vars:replace("gr"..i,grouptable[i].name)
        vars:select("gr"..i.."r")
        vars:replace("gr"..i.."r",grinresult(grouptable[i].name,"р"))
        vars:select("gr"..i.."d")
        vars:replace("gr"..i.."d",grinresult(grouptable[i].name,"д"))
        vars:select("gr"..i.."v")
        vars:replace("gr"..i.."v",grinresult(grouptable[i].name,"в"))
        vars:select("gr"..i.."t")
        vars:replace("gr"..i.."t",grinresult(grouptable[i].name,"т"))
        vars:select("gr"..i.."p")
        vars:replace("gr"..i.."p",grinresult(grouptable[i].name,"п"))
			end
  end
flagassignation = false
end

local function tagging(vd) --функция замены блоков из мада на форматированные блоки
  local num
  local flag = false
  if vd:find(groupflag) then flag = true end
  if flag == true then
  if vd:find(tag) then
  	vd:insertBlock(1)
  	vd:setBlockText(1, "     ")
  	flagassignation = true
  end
  local fs = 1
  for i = vd:getIndex(), vd:size() do
  	if vd:find(tag,i+1) and vd:get(1, "textcolor") == 12 then
  		grouptable[fs] = { name = vd:getBlockText(1):trim(), FKEYS = "F"..fs}
  		vd:insertBlock(1)
      if fs < 10 then
        vd:setBlockText(1, "[ "..grouptable[fs].FKEYS.."]")
      else
        vd:setBlockText(1, "["..grouptable[fs].FKEYS.."]")
      end
  		vd:set(1,"textcolor", vd:get(4, "textcolor"))
  		fs = fs + 1
  	end
  end
  end
end

function gr.before(v, vd) --системная функция получения информации от мада до обработки средствами клиента
  if v ~= 0 then return end
  tagging(vd)
  if flagassignation == true then assignvars() end
end


local w, r, t
local group = {}
local delta
-- часть плагина по работе с окном группы

local function filter(vs)
  if vs:blocks() > 0 then
    local t1 = vs:getBlockText(1)
    if t1:substr(1, 1)== '[' then
      return true, false
    end
      return false, false
  end
end



local function render()
  local x, y = 0, 0
  --if props.pluginsLogWindow() then runCommand("#clear "..props.pluginsLogWindow()) end
  for _,s in ipairs(group) do
    y = hei * _
    for b=1,s:blocks() do
      local t = s:getBlockText(b)
      local c = s:get(b, 'textcolor')
      --log(t.." "..c)
      r:textColor(props.paletteColor(c))
      r:print(x, y, t)
      x = x + r:textWidth(t)
    end
    x = 0
  end
end


function gr.init() --инициализация работы с окном группы
  t = prompt_trigger("\\[.*\\][А-Я].*", filter)
  w = createWindow("Группа",700,195, true)
  --w:setFixedSize(700,195)
  --w:block('left,right,top,bottom')
  if not w then terminate("Окно не создано!") end

  r = w:setRender(render)
  r:setBackground( props.backgroundColor() )
  r:select(props.currentFont())
  hei = r:fontHeight()
  w:hide()

end

function gr.after(v, vd)
  if v ~= 0 then return end
  if t and t:check(vd) then
    group = t.strings
    w:show()
    r:update()
    end
  if vd:find(notingroup) or vd:find(groupDisband) then w:hide() end
end

function gr.disconnect()
  if t then t:disconnect() end
end

return gr
