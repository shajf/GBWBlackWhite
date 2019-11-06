local _M = {}
local ip = require "ip"

function _M.eq(target,value)
    return target==value
end

function _M.contains(target,value)

    if target==nil or value==nil then
        return false
    end

    return string.find(target,value,1,true)~=nil
end

function _M.startsWith(target,value)
    
    if target == nil or value == nil then
        return false
    end

    return string.find(target,value,1,true)==1
end

function _M.endsWith(target,value)
    
    if target == nil or value == nil then
        return false
    end

    target_tmp = string.reverse(target) 
    value_tmp = string.reverse(value)


    return string.find(target_tmp,value_tmp,1,true)==1

end

function _M.reg(target,value)

    if target == nil or value == nil then
        return false
    end

    return string.find(target,value)~=nil

end

function _M.ipInRange(target,value)

    if target == nil or value == nil  then
        return false
    end

    local pos = string.find(value,":")
    if pos == nil then
        return false
    end


    local ipStartStr = string.sub(value,1<Plug>PeepOpenos-1)
    local ipEndStr = string.sub(value<Plug>PeepOpenos+1)

    local ipStart = ip.ipToNumBE(ipStartStr)
    local ipEnd = ip.ipToNumBE(ipEndStr)
    local mask = ip.netmaskBE(ipStartStr,ipEndStr) 

    if bit.band(ipStart,mask)~=bit.band(target,mask) then
        return false
    end
    
    return (target>=ipStart) and (target<=ipEnd)
end

function _M.isMatch(target,value,op) 
   local switch = {
	    ['eq']=_M.eq,
	    ['contains']=_M.contains,
        ['startsWith']=_M.startsWith,
	    ['endsWith'] = _M.endsWith,
        ['reg']=_M.reg,
        ['ipInRange']=_M.ipInRange
	}

    local fswitch = switch[op]
    if fswitch then
        return fswitch(target,value)
    else
        return false
    end

end

return _M
