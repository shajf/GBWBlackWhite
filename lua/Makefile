##########################################################
#Copyright(C) 2012 WAF PROJECT TEAM
#Author(A) shajianfeng
##########################################################

include ../make.include
CFLAGS  = ${BUILD_CFLAGS}  -O0 -rdynamic -D_GNU_SOURCE  
CFLAGS +=  -fPIC

gbw_lua_SOURCES = gbw_mpool.c \
				  gbw_string.c \
				 gbw_tables.c \
				 gbw_lua_engine.c

gbw_lua_OBJECTS = $(patsubst %.c,%.o,$(gbw_lua_SOURCES))
gbw_lua_DEPENDS = $(patsubst %.c,%.d,$(gbw_lua_SOURCES))
gbw_lua_ASMFILE = $(patsubst %.c,%.s,$(gbw_lua_SOURCES))

.PHONY: all clean

all: $(gbw_lua_OBJECTS)

clean:
	@rm -fr $(gbw_lua_OBJECTS) $(gbw_lua_DEPENDS) $(gbw_lua_ASMFILE) $(gbw_lua_package)
	@rm -fr *.d *.o *.s 

