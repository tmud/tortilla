-- autoalias
-- Плагин Tortilla mud client

local autoalias = {}
function autoalias.name() 
  return 'Плагин авто подбора макросов.'
end

function autoalias.description()
  local s = { 'Плагин пробует подобрать макрос(алиас) по набранной команде,',
  'учитывая, что по ошибке могут быть набраны опечатки в начале команды.',
  'Обрабатываются только команды, введенные в командной строке клиента.'
  }
  return table.concat(s, '\r\n')
end

function autoalias.version()
  return '1.0'
end

function autoalias.barcmd(t)
  if #t == 0 then return t end
  if #t == 1 and t[1] == '' then return t end
  local c = t[1]
  for i=1,aliases:size() do
    aliases:select(i)
    local group = aliases:get('group')
    if groups:select(group) then
      local v = groups:get('value')
      if v == '1' then
        local alias = aliases:get('key')
        local pos = c:find(alias)
        if pos then
          local len1 = pos+alias:len()
          local len2 = c:len()+1
          if len1 == len2 or c:substr(len1, 1) == ' ' then 
            local newcmd = alias..c:substr(len1, len2-len1)
            t[1] = newcmd
            return t
          end
        end
      end
    end
  end
end

return autoalias
