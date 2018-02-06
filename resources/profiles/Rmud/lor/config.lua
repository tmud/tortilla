key = "^Вы получили некоторую информацию."

check = function()
  local r1 = createPcre('^Название "(.*)",')
  local r2 = createPcre('^Вы получили некоторую информацию.')
  return function(vs)
    local s = vs:getText()
    if r1:find(s) then return true, r1:get(1) end
    if r2:find(s) then return false end
    return true
  end
end

perpage = 20
