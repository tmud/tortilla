key = "^Вы узнали следующее:"
check = function(create_pcre_func)
  local r1 = create_pcre_func('^Предмет "(.*)",')
  local r2 = create_pcre_func('^За информацию о предмете.*')
  return function(vs)
    local s = vs:getText()
    if r1:find(s) then return vs, r1:get(1) end
    if r2:find(s) then return end
    return refs
  end
end
import = function(create_pcre_func)
  return function(s, vs)
    
  end
end