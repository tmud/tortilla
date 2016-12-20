-- write
-- Плагин Tortilla mud client

local write = {}
function write.name() 
  return 'Запись в файл'
end
function write.description()
  local p = props.cmdPrefix()
  local s = {
    'Плагин записывает текстовую строку в конец файла',
    p.."write файл текст"
  }
  return table.concat(s, "\r\n")
end
function write.version()
  return '-'
end

function write.syscmd(t)
  if t[1] ~= 'write' then
      return t
  end
  if #t == 1 then return end
  local s = ""
  if #t > 2 then
    s = table.concat(t, " ", 3)
  end
  system.appendStringToFile(t[2], s.."\r\n")
end

return write
