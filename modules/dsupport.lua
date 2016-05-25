-- Вспомогательные скрипты для отладки
 
--for x, v in pairs(locals()) do log(x, "=", v) end
--for x,v in pairs(upvalues()) do log("up:", x, "=", v) end

-- список локальных переменных для текущей функции  
function locals()
  local variables = {}
  local idx = 1
  while true do
    local ln, lv = debug.getlocal(2, idx)
    if ln ~= nil then
      variables[ln] = lv
    else
      break
    end
    idx = 1 + idx
  end
  return variables
end

-- список upvalues переменных для текущей функции  
function upvalues()
  local variables = {}
  local idx = 1
  local func = debug.getinfo(2, "f").func
  while true do
    local ln, lv = debug.getupvalue(func, idx)
    if ln ~= nil then
      variables[ln] = lv
    else
      break
    end
    idx = 1 + idx
  end
  return variables
end
