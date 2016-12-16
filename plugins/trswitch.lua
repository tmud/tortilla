-- trswitch
-- Плагин для Tortilla mud client

local trswitch = {}
function trswitch.name() 
  return 'Переключатель работы триггеров'
end
function trswitch.description()
   local s = { 'Плагин добавляет кнопку на панель, которая включает',
   "или выключает триггеры одним кликом." }
   return table.concat(s, "\r\n")
end
function trswitch.version()
  return '-'
end

local function update()
  local mode = props.component("actions")
  if not mode then checkMenu(1)
  else uncheckMenu(1)
  end
end

function trswitch.init()
  if not checkVersion(1,0) then
    terminate('Для работы плагина требуется клиент версии 1.0+')
  end
  addButton("plugins/trswitch.bmp", 0, 1, "Триггеры")
  addMenu("Плагины/Триггеры", 1, "plugins/trswitch.bmp", 0 )
  update()
end

function trswitch.compsupdated()
  update()
end

function trswitch.menucmd()
  local mode = props.component("actions")
  mode = not mode
  setComponent("actions", mode)
  if mode then
    print("[actions] Триггеры включены.")
  else
    print("[actions] Триггеры отключены.")
  end
end

return trswitch
