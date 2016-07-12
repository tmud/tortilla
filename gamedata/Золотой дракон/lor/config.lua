key = "^Вы узнали следующее:"

check = function()
  local r1 = createPcre("^Таймер  : .*")
  local r2 = createPcre("^Предмет '(.*)',")
  return function(vs)
    local s = vs:getText()
    if r1:find(s) then vs:setBlockText(1, "Таймер  : -") return true end
    if r2:find(s) then return true, r2:get(1) end
    return true
  end
end