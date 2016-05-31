key = "^Вы узнали следующее:"
check = function()
  local r1 = createPcre("^Таймер  : .*")
  local r2 = createPcre("^Предмет '(.*)',")
  return function(refs)
    local s = refs:getText()
    if r1:find(s) then refs:setBlockText(1, "Таймер  : -") return refs end
    if r2:find(s) then return refs, r2:get(1) end
    return refs
  end
end