key = "^Предмет '.*', Тип предмета: .*"
check = function()
  local r1 = createPcre("^Предмет '(.*)', Тип предмета: .*")
  local r2 = createPcre('^.* состоянии.')
  local r3 = createPcre('^Вес:.*(Таймер: [0-9]+, ).*')
  return function(vs)
    local s = vs:getText()
    if r1:find(s) then return vs, r1:get(1) end
    if r2:find(s) then return end
    if r3:find(s) then
      local b = r3:first(1)
      local e = r3:last(1)
      local ns = s:substr(1, b-1)
      ns = ns .. s:substr(e, s:len()-e+1)
      vs:setBlockText(1, ns)
    end
    return vs
  end
end
