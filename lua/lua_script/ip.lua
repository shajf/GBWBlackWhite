local _M = {}

function _M.ipToNumLE(ip)

	local num = 0
	if ip and type(ip)=="string" then
		local p1,p2,p3,p4 = ip:match("(%d+)%.(%d+)%.(%d+)%.(%d+)" )

		num = 2^24*p1 + 2^16*p2 + 2^8*p3 + p4

	end

    return num
end

function _M.ipToNumBE(ip)

	local num = 0
	if ip and type(ip)=="string" then
		local p1,p2,p3,p4 = ip:match("(%d+)%.(%d+)%.(%d+)%.(%d+)" )

		num = 2^24*p4 + 2^16*p3 + 2^8*p2 + p1

	end

    return num
end

function _M.numToStrLE(n)

   if n then
		n = tonumber(n)
		local n1 = math.floor(n / (2^24)) 
		local n2 = math.floor((n - n1*(2^24)) / (2^16))
		local n3 = math.floor((n - n1*(2^24) - n2*(2^16)) / (2^8))
		local n4 = math.floor((n - n1*(2^24) - n2*(2^16) - n3*(2^8)))
    	return n1.."."..n2.."."..n3.."."..n4 
	end
	return "0.0.0.0" 
end

function _M.numToStrBE(n)

   if n then
		n = tonumber(n)
		local n1 = math.floor(n / (2^24)) 
		local n2 = math.floor((n - n1*(2^24)) / (2^16))
		local n3 = math.floor((n - n1*(2^24) - n2*(2^16)) / (2^8))
		local n4 = math.floor((n - n1*(2^24) - n2*(2^16) - n3*(2^8)))
    	return n4.."."..n3.."."..n2.."."..n1 
	end
	return "0.0.0.0" 
end

function _M.netmaskLE(ipStart,ipEnd)

    local nt = {[1]="255.0.0.0",[2]="255.255.0.0",[3]='255.255.255.0',[4]='255.255.255.255'}
    local ipToNumLE = _M.ipToNumLE

    if ipStart == nil or ipEnd == nil then
        return ipToNumLE("255.255.255.255")
    end

	local a1,a2,a3,a4 = ipStart:match("(%d+)%.(%d+)%.(%d+)%.(%d+)" )
	local b1,b2,b3,b4 = ipEnd:match("(%d+)%.(%d+)%.(%d+)%.(%d+)" )
    local n = 0
    
    if a1==b1 then
        n=n+1
    else
        return ipToNumLE("255.255.255.255")
    end
    
    if a2==b2 then
        n=n+1
    else
        return ipToNumLE(nt[n])
    end
    
    if a3==b3 then
        n=n+1
    else
        return ipToNumLE(nt[n])
    end

    if a4==b4 then
        n=n+1
    else
        return ipToNumLE(nt[n])
    end

    return ipToNumLE(nt[n])
end

function _M.netmaskBE(ipStart,ipEnd)

    local nt = {[1]="255.0.0.0",[2]="255.255.0.0",[3]='255.255.255.0',[4]='255.255.255.255'}
    local ipToNumBE = _M.ipToNumBE
    
    if ipStart == nil or ipEnd == nil then
        return ipToNumBE("255.255.255.255")
    end

	local a1,a2,a3,a4 = ipStart:match("(%d+)%.(%d+)%.(%d+)%.(%d+)" )
	local b1,b2,b3,b4 = ipEnd:match("(%d+)%.(%d+)%.(%d+)%.(%d+)" )
    local n = 0
    
    if a1==b1 then
        n=n+1
    else
        return ipToNumBE("255.255.255.255")
    end
    
    if a2==b2 then
        n=n+1
    else
        return ipToNumBE(nt[n])
    end
    
    if a3==b3 then
        n=n+1
    else
        return ipToNumBE(nt[n])
    end

    if a4==b4 then
        n=n+1
    else
        return ipToNumBE(nt[n])
    end

    return ipToNumBE(nt[n])
end

return _M
