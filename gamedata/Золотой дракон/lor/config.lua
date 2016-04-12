key = "^Вы узнали следующее:"
check = function(create_pcre_func)
  local r1 = create_pcre_func("^Таймер  : .*")
  local r2 = create_pcre_func("^Предмет '(.*)',")
  return function(refs)
    local s = refs:getText()
    if r1:find(s) then refs:setBlockText(1, "Таймер  : -") return refs end
    if r2:find(s) then return refs, r2:get(1) end
    return refs
  end
end