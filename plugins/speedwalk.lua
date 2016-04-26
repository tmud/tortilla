-- speedwalk
-- Плагин для Tortilla mud client
local speedwalk = {}

function speedwalk.name()
  return 'Маршуты и перемещения'
end
function speedwalk.description()
  local s = { 'Плагин позволяет быстро перемещатся по миру по заданным маршрутам(speedwalk).',
  'Можно сохранять маршуты в базу и использовать позднее. Помогает находить обратную дорогу.',
  'Работает с мадами с 6 направлениями для передвижений - север,юг,запад,восток,вверх,вниз.',
  '#swalk start - начать запись в память клиента перемещения от текущей комнаты.',
  '#swalk stop - остановить запись, забыть маршрут.',
  '#swalk save name - остановить запись, сохранить маршрут в базу под именем name.',
  '#swalk return - вернуться по маршруту. Должна идти запись маршрута.',
  '#swalk return name - не должно быть записи. Вернутся обратно по маршуту из базы с именем name.',
  '#swalk go name - не должно быть записи. Идти по маршруту из базы по имени name.',
  '#swalk play path - не должно быть записи. Идти по маршруту - одна буква - одно направление.',
  '#swalk list - показать список маршрутов в базе.',
  '#swalk delete name - удалить маршрут из базы.',  
  '#swalk show name - показать маршрут на экране.'
 }
  return table.concat(s, '\r\n')
end
function speedwalk.version()
  return '1.0'
end

function speedwalk.init()
  addCommand("swalk")
end

function speedwalk.syscmd(t)
  if t[1] ~= 'swalk' then
   return t
  end
  local c = t[2]
  if not c then
    print(props.getPrefix().."swalk start|stop|save|return|go|play|list|delete|show")
	return {}
  end
  
  return {}
end

function speedwalk.gamecmd(t)
  return t
end

return speedwalk
