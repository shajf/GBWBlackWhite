local ffi = require "ffi"
local file = require "file"
local op = require "op"

local cast = ffi.cast
local ctostring = ffi.string
local getfenv = getfenv

local jsonFile = nil
local rules = nil


ffi.cdef[[
     typedef struct gbw_http_session_t gbw_http_session_t;
     struct gbw_http_session_t {

		 uint32_t srcIP;
		 uint32_t dstIP;
         const char *method;
		 const char *uri;
		 const char *host;
     };

     typedef struct gbw_http_match_result_t gbw_http_match_result_t;
     
     struct gbw_http_match_result_t {

         int isMatch;
         long ruleID;
     };
 ]]

 local function getTarget(session,name)

     if name == nil then
         return nil
     end

     local sessionTable = {
         ["srcIP"] = session.srcIP,
         ["dstIP"] = session.dstIP,
         ["method"] = ctostring(session.method),
         ["host"] = ctostring(session.host),
         ['uri'] = ctostring(session.uri)
     }

     return session[name]
 end

 local function doRuleMatch(session,rule)

     local isAnd = rule['isAnd']
     local items = rule['items']
     
     if items ==nil then
         return false
     end

     for _,v in pairs(items) do

         local target = getTarget(session,v['target'])
         local ops = v['op']
         local value = v['value']

         if target and ops and value then
             
             local m = op.isMatch(target,value,ops)
             if isAnd and (not m) then
                 return false
             end

             if (not isAnd) and m then
                 return true
             end
         end
     end
     
     if isAnd then
         return true
     else
         return false
     end
 end
 
 local function doMatch(session,result)

     for _,v in pairs(rules) do

         if doRuleMatch(session,v) then
             result.isMatch = 1
             result.ruleID = v['ruleID']
             return true
         end
     end

     result.isMatch = 0
     result.ruleID = 0
     return false
 end
 
 function wbw_init()

     jsonFile = ctostring(getfenv(0).__wbw_jsonFile)

     if jsonFile == nil then
         return error("Must specify web back white list jsonFile!")
     end

     local json = file.parseJson(jsonFile)
     rules = json['rules']
 end

 function wbw_match()

     local matchResult = cast("gbw_http_match_result_t*",getfenv(0).__wbw_match_result)
     local httpSession = cast("gbw_http_session_t*",getfenv(0).__wbw_http_session)

     if not matchResult or not httpSession then
         return 0
     end

     if not rules then
         return 0
     end

     if doMatch(httpSession,matchResult) then
         return 1
     end

     return 0
 end

 function wbw_fin()
     -- do nothing
 end

