local miner = {}

function miner.name()
    return 'Игра сапер'
end

function miner.description()
  return 'Реализация игры сапер'
end

function miner.version()
    return '-'
end
local w, r, p, br, br2, num1, num2, num3, num4, num5, num6, num7, num8  --окно, рендер, ручка
local maincolor = {r=122, g=122, b=122}
local flagcolor = {r=240}
local num1color = {r=0, g=115, b=255}
local num2color = {g=255}
local num3color = {r=255, g=255}
local num4color = {r=255, g=150, b=0}
local num5color = {g=255, b=255}
local num6color = {r=255, b=255}
local num7color = {g=255, b=170}
local num8color = {r=255}
local m, n = 200, 200
local shag = 20
local gap = 10
local midtext, midtext2
local Mines = {}
local GridCoordI = {}
local GridCoordJ = {}
local MinesCount = 20
local minerWindow = false
local state = true
local GridState = {}
local FlagState = {}
local xm, ym, sectorcoordX, sectorcoordY
local stateClick = false

local function genGrid()
  local xx = 0
  local yy = 0
  for i = gap, n, shag do
    xx = xx + 1
    --print("xx: "..xx)
    for j = gap, m, shag do
      yy = yy + 1
      --print("yy: "..yy)
      if GridState[xx][yy] ~= "1" then
        if FlagState[xx][yy] == "0" then
          r:select(br)
          r:solidRect{x=i, y=j, width=shag-1, height=shag-1}
        else
          r:select(br2)
          r:solidRect{x=i, y=j, width=shag-1, height=shag-1}
        end

      else
        r:rect{x=i, y=j, width=shag-1, height=shag-1}
      end
    end
    yy = 0
  end
end

local function genMine()
  -- Заполняем нулями массив
  for i = 1, 10 do
    Mines[i] = {}
    GridState[i] = {}
    FlagState[i] = {}
    for j = 1, 10 do
      Mines[i][j] = "0"
      GridState[i][j] = "0"
      FlagState[i][j] = "0"
    end
  end
  -- Добавляем в матрицу мины, проверяем есть мина на текущем шаге? если есть - шаг повторяем
  local tmpcounter = 1
  repeat
    local ii = math.random(1,10)
    local yy = math.random(1,10)
    if Mines[ii][yy] ~= "M" then
      Mines[ii][yy] = "M"
      tmpcounter = tmpcounter + 1
    else
  end
  until tmpcounter == MinesCount + 1

  -- Ищем мины на каждом шагу цикла, наращиваем счетчики
  for i = 1, 10 do
    for j = 1, 10 do
        if Mines[i][j] ~= "M" then
        if j-1 ~= 0 then
          if Mines[i][j-1] == "M" then
            Mines[i][j] = tostring(tonumber(Mines[i][j]) + 1)
          end
        end
        if j+1 ~= 11 then
          if Mines[i][j+1] == "M" then
            Mines[i][j] = tostring(tonumber(Mines[i][j]) + 1)
          end
        end
        if i-1 ~= 0 then
          if Mines[i-1][j] == "M" then
            Mines[i][j] = tostring(tonumber(Mines[i][j]) + 1)
          end
        end
        if i+1 ~= 11 then
          if Mines[i+1][j] == "M" then
            Mines[i][j] = tostring(tonumber(Mines[i][j]) + 1)
          end
        end
        if i+1 ~= 11 and j+1 ~= 11 then
          if Mines[i+1][j+1] == "M" then
            Mines[i][j] = tostring(tonumber(Mines[i][j]) + 1)
          end
        end
        if i-1 ~= 0 and j-1 ~= 0 then
          if Mines[i-1][j-1] == "M" then
            Mines[i][j] = tostring(tonumber(Mines[i][j]) + 1)
          end
        end
        if i-1 ~= 0 and j+1 ~= 11 then
          if Mines[i-1][j+1] == "M" then
            Mines[i][j] = tostring(tonumber(Mines[i][j]) + 1)
          end
        end
        if i+1 ~= 11 and j-1 ~= 0 then
          if Mines[i+1][j-1] == "M" then
            Mines[i][j] = tostring(tonumber(Mines[i][j]) + 1)
          end
        end
        end
    end
  end
  -- for i = 1, 10 do
  --   for j = 1, 10 do
  --     log("i= "..i.." j= "..j.." Mines["..i.."]["..j.."]= "..Mines[i][j])
  --
  --   end
  -- end
end

local function checkState()
  local counter = 0
  for i = 1, 10 do
    for j = 1, 10 do
      if Mines[i][j] == "M" and FlagState[i][j] == "F" then counter = counter + 1 end
    end
  end
  if counter == MinesCount then
    return true
  else
    return false
  end
end

local function render()
  genGrid()
  if checkState() then print("WIN") end
  for i = 1, 10 do
    for j = 1, 10 do
      -- r:print(shag*i/2 - midtext2 + gap, shag*j/2 - midtext + gap, Mines[i][j])
      if Mines[i][j] ~= "M" and Mines[i][j] ~= "0" then
        if Mines[i][j] == "1" and GridState[i][j] == "1" then
          r:textColor(num1color)
          r:print(shag*i-midtext2-1, shag*j-midtext-1, Mines[i][j])
        elseif Mines[i][j] == "2" and GridState[i][j] == "1" then
          r:textColor(num2color)
          r:print(shag*i-midtext2-1, shag*j-midtext-1, Mines[i][j])
        elseif Mines[i][j] == "3" and GridState[i][j] == "1" then
          r:textColor(num3color)
          r:print(shag*i-midtext2-1, shag*j-midtext-1, Mines[i][j])
        elseif Mines[i][j] == "4" and GridState[i][j] == "1" then
          r:textColor(num4color)
          r:print(shag*i-midtext2-1, shag*j-midtext-1, Mines[i][j])
        elseif Mines[i][j] == "5" and GridState[i][j] == "1" then
          r:textColor(num5color)
          r:print(shag*i-midtext2-1, shag*j-midtext-1, Mines[i][j])
        elseif Mines[i][j] == "6" and GridState[i][j] == "1" then
          r:textColor(num6color)
          r:print(shag*i-midtext2-1, shag*j-midtext-1, Mines[i][j])
        elseif Mines[i][j] == "7" and GridState[i][j] == "1" then
          r:textColor(num7color)
          r:print(shag*i-midtext2-1, shag*j-midtext-1, Mines[i][j])
        elseif Mines[i][j] == "8" and GridState[i][j] == "1" then
          r:textColor(num8color)
          r:print(shag*i-midtext2-1, shag*j-midtext-1, Mines[i][j])
        end

      end

    end
  end
end




local function clearGrid(x, y)
    if GridState[x][y] == "1" then return end
    if x >= 1 and x <= 10 and y >= 1 and y <= 10 then
      if Mines[x][y] == "0" then
        GridState[x][y] = "1"
        if x > 1 then clearGrid(x-1, y) end
        if x < 10 then clearGrid(x+1, y) end
        if y > 1 then clearGrid(x, y-1) end
        if y < 10 then clearGrid(x, y+1) end
        if x > 1 and y > 1 then clearGrid(x-1, y-1) end
        if x < 10 and y < 10 then clearGrid(x+1, y+1) end
        if x > 1 and y < 10 then clearGrid(x-1, y+1) end
        if x < 10 and y > 1 then clearGrid(x+1, y-1) end
      else
        if Mines[x][y] ~= "M" and Mines[x][y] ~= "0" then GridState[x][y] = "1" end

      end
    end
end


local mouse = {}
function mouse.left(c, x, y)
    if x>gap and x<gap+m and y>gap and y<gap+n then
      xm = math.floor((x+gap)/shag)
      ym = math.floor((y+gap)/shag)
      sectorcoordX = xm * shag + gap
      sectorcoordY = ym * shag + gap
      if c == "down" then
        if FlagState[xm][ym] ~= "F" then
          if Mines[xm][ym] == "M" then
            print("BOOM!")

            -- Реализация механизма "Показать мины после подрыва"
            -- Нет возможности остановки потока для вывода мин + запрос нажатия клавишы
            -- wait не реализован


            -- for i = 1, 10 do
            --   for j = 1, 10 do
            --     if Mines[i][j] ~= "M" then
            --       GridState[i][j] = "0"
            --     else
            --       GridState[i][j] = "1"
            --     end
            --
            --   end
            -- end
            -- r:update()



            genMine()

          elseif Mines[xm][ym] == "0" then

            clearGrid(xm,ym)-- функция очистки поля до цифр
            r:update()
          else
            GridState[xm][ym] = "1"
          end
          -- print(ym.." Координата Y: "..y)
          -- print(xm.." Координата Х: "..x)
          -- local tmp = ""
          -- for j = 1, 10 do
          --   for i = 1, 10 do
          --     tmp = tmp..GridState[i][j]
          --   end
          --   print(tmp)
          --   tmp = ""
          -- end
          r:update()
        end
      end
    end
end

function mouse.right(c, x, y)
  if x>gap and x<gap+m and y>gap and y<gap+n then
    xm = math.floor((x+gap)/shag)
    ym = math.floor((y+gap)/shag)
    sectorcoordX = xm * shag + gap
    sectorcoordY = ym * shag + gap
    if c == "down" then
      if FlagState[xm][ym] == "0" then
        FlagState[xm][ym] = "F"
      else
        FlagState[xm][ym] = "0"
      end
    end
    -- print(ym.." Координата Y: "..y)
    -- print(xm.." Координата Х: "..x)
    -- local tmp = ""
    -- for j = 1, 10 do
    --   for i = 1, 10 do
    --     tmp = tmp..FlagState[i][j]
    --   end
    --   print(tmp)
    --   tmp = ""
    -- end
    r:update()
  end
end


-- Добавляем кнопку на панель, отслеживаем статус нажатия, меняем иконку нажатия

local function update ()
  if minerWindow then checkMenu(1)
  else uncheckMenu(1)
  end
end

function miner.menucmd(id)
  if id == 1 then
    if minerWindow then
      minerWindow = false
      w:hide()
      update()
    else
      minerWindow = true
      w:show()
      update()
    end
  end
end

-- Генерация поля мин, цифр у мин, пустых ячеек






function miner.init()
  w = createWindow("Сапер",m+2*gap,n+2*gap, false)
  w:setFixedSize(m+2*gap,n+2*gap)
  if not w then
    terminate("Error on window creating!")
  end
  r = w:setRender(render)
  r:setBackground(props.backgroundColor())
  p = r:createPen{width=2, color=maincolor}
  br = r:createBrush{color=maincolor}
  br2 = r:createBrush{color=flagcolor}
  num1 = r:createPen{color=num1color}
  num2 = r:createPen{color=num2color}
  num3 = r:createPen{color=num3color}
  num4 = r:createPen{color=num4color}
  num5 = r:createPen{color=num5color}
  num6 = r:createPen{color=num6color}
  num7 = r:createPen{color=num7color}
  num8 = r:createPen{color=num8color}
  r:select(p)
  r:select(props.currentFont())
  r:select(br)
  w:hide()
  midtext = r:fontHeight() / 2
  midtext2 = r:textWidth("?") / 2
  addButton("plugins/miner.bmp", 16, 1, "Сапер")
  w:attachMouse(mouse)
  genMine()
end

-- Функционал кнопки Х у окна (прячем окно игры)

function miner.closewindow()
  if w then
    minerWindow = false
    update()
    w:hide()
  end
end


return miner
