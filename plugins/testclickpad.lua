local testclickpad = {}

function testclickpad.name()
  return 'Тестирование api плагина clickpad'
end
function testclickpad.description()
  return 'Плагин для тестирования api плагина clickpad.'
end
function testclickpad.version()
  return '-'
end

function testclickpad.menucmd()
  clickpad.set(1,2,{text="2x", command="#test2"})
  clickpad.set(1,3,{image="64-1\\diablo3.png", imagex=7, imagey=1, command="#test3"})
  clickpad.clear(1,1)
  clickpad.update(1,4,{command="#test4", text="4a"})
  clickpad.set(1,5,nil)
  clickpad.show(2,7)
end

function testclickpad.init()
  local c = clickpad
  if not c then 
    print('clickpad not found')
    return
  end
  print('test clickpad')
  local t = clickpad.get(1, 1)
  print (t.text)
  print (t.command)
  print (t.image)
  clickpad.set(1,1,{text="1", command="#test"})
  clickpad.set(1,2,{text="2", command="#test"})
  clickpad.set(1,3,{text="3", command="#test"})
  clickpad.set(1,4,{text="4", command="#test"})
  clickpad.set(1,5,{text="5", command="#test"})
  clickpad.show(1,6)
  
  addMenu("Clickpad/Test", 1)
  
end

return testclickpad
