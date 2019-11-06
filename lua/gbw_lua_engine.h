/*
 * =====================================================================================
 *
 *       Filename:  gbw_lua_engine.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/12/2018 11:23:43 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef GBW_LUA_ENGINE_H
#define GBW_LUA_ENGINE_H

typedef struct gbw_lua_engine_t gbw_lua_engine_t;

#include "gbw_lua.h"
#include "gbw_mpool.h"
#include "gbw_tables.h"


struct gbw_lua_engine_t {

    lua_State *lua_state;

    gbw_pool_t *mp;

    const char *lua_path;
    const char *lua_cpath;

    const char *lua_init_fun;
    const char *lua_match_fun;
    const char *lua_fin_fun;

    int is_cache;

    const char *lua_fname;

    gbw_array_header_t *chunk_parts;

};

extern gbw_lua_engine_t * gbw_lua_engine_create(gbw_pool_t *mp,const char *lua_path,
        const char *lua_cpath,
        const char *lua_init_fun,
        const char *lua_match_fun,
        const char *lua_fin_fun,
        int is_cache,
        const char *lua_fname,
        const char *data_key,
        void *data);


extern int gbw_lua_engine_run_match(gbw_lua_engine_t *engine,const char *idata_key,void *idata,
        const char *odata_key,void *odata);


extern void gbw_lua_engine_fin(gbw_lua_engine_t *engine);

#define gbw_lua_engine_udata_set(lua_engine,key,data) do { \
    lua_pushlightuserdata(lua_engine->lua_state,data);    \
    lua_setglobal(lua_engine->lua_state, key);            \
}while(0)

#endif /* GBW_LUA_ENGINE_H */
