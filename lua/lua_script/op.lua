local _M = {}

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

return _M
