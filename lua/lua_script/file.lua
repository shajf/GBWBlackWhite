local _M = {}
local fopen = io.open
local cjson = require "cjson"

function _M.readFile(fname)

	local file = fopen(fname,"r")

	if file == nil then
		return nil
    end

    local content = file:read("*a");
    file:close()

    return content
end

function _M.parseJson(fname) 
	
	if fname == nil or string.len(fname) == 0 then
		return nil
	end
	
	local content = _M.readFile(fname) 
	if content == nil then
		return nil
   	end

	return cjson.decode(content)
end

return _M
