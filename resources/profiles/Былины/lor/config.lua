key = "^Вы узнали следующее:"
check = function(create_pcre_func)
  local r1 = create_pcre_func('^Предмет "(.*)",')
  local r2 = create_pcre_func('^За информацию о предмете.*')
  return function(vs)
    local s = vs:getText()
    if r1:find(s) then return vs, r1:get(1) end
    if r2:find(s) then return end
    return vs
  end
end
import = function(create_pcre_func)
  local r1 = create_pcre_func('^Предмет "(.*)",')
  local r2 = create_pcre_func('^(Неудобен : |Материал : |Недоступен : |Имеет экстрафлаги: |Накладывает на вас аффекты: )(.*)')
  local r3 = create_pcre_func('^( +)(.*)')
  return function(s, vs)
    if r1:find(s) then
      vs:setBlocksCount(1)
      vs:setBlockText(1, s)
      return true, r1:get(1)
    end
    if r2:find(s) or r3:find(s) then
      vs:setBlocksCount(2)
      local r = r2:size() > 0 and r2 or r3
      vs:setBlockText(1, r:get(1))
      vs:setBlockText(2, r:get(2))
      vs:set(2, 'textcolor', 6)
    else
      vs:setBlocksCount(1)
      vs:setBlockText(1, s)
    end
    return true
  end
end