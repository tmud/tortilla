key = "^Вы узнали следующее:"

check = function()
  local r1 = createPcre('^Предмет "(.*)",')
  local r2 = createPcre('^За информацию о предмете.*')
  local r3 = createPcre('^Вы узнали следующее:')
  return function(vs)
    local s = vs:getText()
    if r1:find(s) then return true, r1:get(1) end
    if r2:find(s) or r3:find(s) then return false end
    return true
  end
end

import_key = '^Предмет "(.*)",'

import = function()
  local r1 = createPcre('^Предмет "(.*)",')
  local r2 = createPcre('^(Неудобен : |Материал : |Недоступен : |Имеет экстрафлаги: |Накладывает на вас аффекты: )(.*)')
  local r3 = createPcre('^( +)(.*)')
  return function(vs)
    local s = vs:getText()
    if r1:find(s) then return true, r1:get(1) end
    if r2:find(s) or r3:find(s) then
      vs:setBlocksCount(2)
      local r = r2:size() > 0 and r2 or r3
      vs:setBlockText(1, r:get(1))
      vs:setBlockText(2, r:get(2))
      vs:set(2, 'textcolor', 6)
    end
    return true
  end
end

tegs = function()
  local r1 = createPcre("Материал : (.*)")
  return function(s)
    if r1:find(s) then return r1:get(1) end
  end
end