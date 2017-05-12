function make_class_object(class, object)
  object = object or {}
  local interface = {}
  for k,v in pairs(class) do
    if k ~= 'new' then
      local f = class[k]
      interface[k] = function(p, ...)
        if p == interface then
          return f(object, ...)       -- object:method call
        else
          return f(object, p, ...)    -- object.method call
        end
      end
    end
  end
  setmetatable(object, { __index = class })
  return setmetatable(interface, {} )
end
