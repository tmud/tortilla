-- testrender
-- Плагин Tortilla mud client

local testrender = {}
function testrender.name() 
  return 'Плагин для тестирования рендера'
end

function testrender.description()
  return 'Плагин используется для тестирования ренедра в окно в дополнительное окно.'
end

function testrender.version()
  return '-'
end

local w, r
local pen1, pen2, pen3, brush1, brush2, brush3
local function render()
  r:select(pen1)
  r:rect{10, 20, 130, 130}
  r:select(brush1)
  r:solidRect{50, 50, 220, 150}
  r:select(pen2)
  r:point(200, 200)
  r:lineTo(0, 0)
  r:solidEllipse{80, 70, 280, 190}
  r:solidEllipse{90, 120, 180, 290, color={r=40, g=60, b=80 } }
  r:ellipse{110, 150, 180, 130 }
  r:select(pen3)
  r:circle(120, 120, 120)
  r:rect{0, 0, 240, 240}
  r:select(brush2)
  r:solidCircle(140, 160, 20)
  r:select(brush3)
  r:select(pen1)
  r:solidCircle(220, 220, 60)
  r:select(brush2)
  r:solidCircle{x=120, y=270, r=80, color={ r=255, g=255, b=0} }
end

function testrender.init()
  w = createWindow('Тесты рендера', 300, 300, true)
  if not w then
    terminate('Ошибка при создании окна.')
  end
  w:block('left,right,top,bottom')
  r = w:setRender(render)
  r:setBackground( props.backgroundColor() )
  r:select(props.currentFont())
  pen1 = r:createPen{ color={r=255,g=20,b=30} }
  pen2 = r:createPen{ color={r=0,g=255,b=0} }
  pen3 = r:createPen{ color={r=0,g=0,b=255} }
  brush1 = r:createBrush{ color={r=100,g=120,b=30} }
  brush2 = r:createBrush{ color={r=140,g=160,b=130} }
  brush3 = r:createBrush{ color={r=140,g=160,b=130}, style="diagcross" }
end

return testrender