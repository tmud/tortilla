local ingrs_level_s = { 'наилучшую', 'отличную', 'хорошую', 'среднюю', 'посрдественную', 'худшую' }
local ingrs_level_m = { 'наилучшие', 'отличные', 'хорошие', 'средние', 'посрдественные', 'худшие' }

local inrgs = {
  { 'трава', 'траву' }
}


local ingr_vetka = {

}


local cmdrxp, cmdrxp2, decl
local autosbor = {}
function autosbor.init()
  --[[if not extra or not extra.declension then
    terminate("Для работы плагина нужен модуль extra.declension")
  end
  decl =  extra.declension()
  ]]
  cmdrxp = createPcre('сбор(.*)')
  cmdrxp2 = createPcre('^(_[^+-]+)?([+-])$')
end

local function runcmd(p)
  if p == '' then
  -- состояние сбора
    return true
  end
  if not cmdrxp2:find(p) then
    return
  end
  if cmdrxp:get(1) == '' then
    -- весь сбор
    return true
  end
  
end

function autoresc.gamecmd(t)
  if cmdrxp:find(t[1]) then
    if #t == 1 then
      if runcmd(cmdrxp:get(1)) then return end
    end
  end
  return t
end

function autosbor.name()
  return 'Автосбор для Былин'
end
function autosbor.description()
  local s = {
  'Плагин автоматически собирает ингридиенты (траву, грибы, ветки, жидкости, крупы, металлы,',
  'минералы, овощи, посуду, отвары, ягоды) различного качества.',
  'Команда: сбор[_что][качество]+-',
  'Вывод текущего статуса автосбора: сбор',
  'Включить или выключить автосбор полностью: сбор+, сбор-',
  'Включить или выключить сбор отдельного игридиента: сбор_трава+, сбор_ветки-',
  'Включить или выключить сбор отдельного игридиента и качества: сбор_ягоды3+, сбор_грибы2-',
  'Включить или выключить сбор отдельного игридиента всех качеств: сбор_минералы_все+, сбор_овощи_все-',
  }
  return table.concat(s, '\r\n')
end
function autosbor.version()
  return '-'
end

return autosbor
