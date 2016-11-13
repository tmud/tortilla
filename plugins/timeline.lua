-- timeline
-- Плагин для Tortilla mud client

-- цвет часов (см. справку #help color)
local color = 'green'

local timeline = {}
function timeline.name()
  return 'Время в каждую строчку'
end
function timeline.description()
  return 'Плагин добавляет текущее время в каждую строчку игровой информации.'
end
function timeline.version()
  return '1.0'
end

local teg_text_color, teg_bkg_color
local function updateColors()
  if type(color) == 'string' then
    teg_text_color, teg_bkg_color = translateColors( color, props.paletteColor(7), props.backgroundColor() )
  end
  if not teg_text_color then
    teg_text_color = props.paletteColor(7)
    teg_bkg_color = props.backgroundColor()
  end
end

function timeline.init()
  if not system or not system.getTime then
    terminate("Не загружен модуль system.getTime()")
  end
  updateColors()
end

function timeline.propsupdated()
  updateColors()
end

function timeline.after(v, vd)
  if v ~= 0 then return end
  for i=1,vd:size() do
    vd:select(i)
    if i==1 and vd:isFirst() then goto next end
    if vd:getTextLen() == 0 or vd:isSystem() then goto next end
    if vd:insertBlock(1) then
      local t = vd:getBlockText(2)
      vd:setBlockText(2, ' '..t)
      local s,m,h = system.getTime()
      local ss = s < 10 and ":0"..s or ":"..s
      local ms = m < 10 and ":0"..m or ":"..m
      vd:setBlockText(1, ""..h..ms..ss)
      vd:set(1, 'exttextcolor', teg_text_color)
      vd:set(1, 'extbkgcolor', teg_bkg_color)
    end
    ::next::
  end
end

return timeline