local miner = {}

function miner.name()
    return 'Игра сапер'
end

function miner.description()
  local t = {
   'Реализация игры сапер.',
   'Левая кнопка мыши - открыть клетку.',
   'Правая кнопка мыши - пометить клетку с предполагаемой миной.',
   'Цифра в клетке (1-8) показывает количество мин вокруг нее.',
   'Цель игры - пометить все мины и не подорваться.',
   'Автор - Саликов Артем'
   }
  return table.concat(t, '\r\n')
end

function miner.version()
    return '1.01'
end

local w, r, p, br, br2, msgwnd --окно, рендер, и др.
local maincolor = {r=122, g=122, b=122}
local msgwndcolor = {r=160, g=160, b=160}
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
local state = true
local GridState = {}
local FlagState = {}
local xm, ym, sectorcoordX, sectorcoordY
local stateClick = false
local lose = false

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
  lose = false
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
  --   end
  -- end
end

local function checkWin()
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

local function checkLose()
  return lose
end

local function render()
  genGrid()
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
  local text = ''
  if checkWin() then
    text = 'WIN'
  end
  if checkLose() then
    text = 'BOOM!'
  end
  if text ~= '' then
    local w = r:width()
    local h = r:height()
    local x = math.modf((w - 100) / 2)
    local y = math.modf((h - 48) / 2)
    local pos = { x, y, x+100, y+48 }
    local dx = (100 - r:textWidth(text)) / 2
    r:select(msgwnd)
    r:solidRect(pos)
    r:textColor(flagcolor)
    pos[1] = x + dx
    r:print(pos, text)
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
  if c ~= 'down' then return end
    if checkLose() or checkWin() then
      genMine()
      r:update()
      return
    end
    if x>gap and x<gap+m and y>gap and y<gap+n then
      xm = math.floor((x+gap)/shag)
      ym = math.floor((y+gap)/shag)
      sectorcoordX = xm * shag + gap
      sectorcoordY = ym * shag + gap
      if c == "down" then
        if FlagState[xm][ym] ~= "F" then
          if Mines[xm][ym] == "M" then
            lose = true
          elseif Mines[xm][ym] == "0" then
            clearGrid(xm,ym)-- функция очистки поля до цифр
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
  if c ~= 'down' then return end
  if checkLose() or checkWin() then
    return
  end
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
local function update (minerWindow)
  if minerWindow then 
    w:show()
    checkMenu(1)
  else
    w:hide()
    uncheckMenu(1)
  end
end

function miner.menucmd(id)
  if id == 1 then
    update(not w:isVisible())
  end
end

-- Генерация поля мин, цифр у мин, пустых ячеек
function miner.init()
  if getDpi then
    shag = shag * getDpi()
    m = m * getDpi()
    n = n * getDpi()
    gap = gap * getDpi()
  end
  local ww = m+2*gap
  local h = n+2*gap
  w = createWindow("Сапер",ww,h)
  if not w then
    terminate("Error on window creating!")
  end
  w:setFixedSize(ww,h)
  r = w:setRender(render)
  r:setBackground(props.backgroundColor())
  p = r:createPen{ width=2, color=maincolor }
  br = r:createBrush{ color=maincolor }
  br2 = r:createBrush{ color=flagcolor }
  msgwnd = r:createBrush{ color=msgwndcolor }
  r:select(p)
  r:select(props.currentFont())
  r:select(br)
  midtext = r:fontHeight() / 2
  midtext2 = r:textWidth("?") / 2
  addButton("plugins/miner.bmp", 16, 1, "Сапер")
  w:attachMouse(mouse)
  if w:isVisible() then checkMenu(1) end
  genMine()
end

-- Функционал кнопки Х у окна (прячем окно игры)
function miner.closewindow()
  if w then
    update(false)
  end
end

return miner
