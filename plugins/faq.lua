-- faq
-- Плагин для Tortilla mud client

local texts = {
'В клиенте есть подсказки. Их можно прочитать с помощью команды '..props.cmdPrefix()..'faq.',
'Для подключения к игровому серверу (по умолчанию) нужно нажать клавишу F12.',
{ 'Чтобы скопировать текст из окна клиента в буфер обмена нужно нажать и', 
  'держать кнопку Shift, далее мышкой и ее левой кнопкой выделить нужный учаcток текста.',
  'После отпускания кнопки мыши текст будет скопирован в буфер обмена.' },
'Если у вас текст отображается с разрывами, то проверьте, включен ли режим га(автозавершения) в маде.',
}


local faq = {}
function faq.name() 
  return 'Плагин подсказок'
end

function faq.description()
  local s  = {
  'Плагин пишет подсказки о пользовании клиентом при каждом запуске.',
  'Добавляет команду '..props.cmdPrefix()..'faq, которая позволяет прочитать следующую подсказку.'
  }
  return table.concat(s, '\r\n')
end

function faq.version()
  return '1.0'
end

local index = 1
function nextfaq()
  if not texts or #texts == 0 then return end
  if not index or index < 1 or index > #texts then index = 1 end
  local t = texts[index]
  if type(t) == 'table' then
    print('FAQ('..index..'/'..#texts..') '..t[1])
    for i=2,#t do print(t[i]) end
  else
    print('FAQ('..index..'/'..#texts..') '..t)
  end
  index = index + 1
end

function faq.init()
  local t = loadTable('config.xml')
  if t and t.index then
    local value = tonumber(t.index)
    if value then index = value end
  end
  addCommand('faq')
  nextfaq()
end

function faq.release()
  local t = { index = index }
  saveTable(t, 'config.xml')
end

function faq.syscmd(t)
  if t[1] ~= 'faq' then return t end
  nextfaq()
end

return faq
