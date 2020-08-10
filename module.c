#include "module.h"
#include "dsf.h"
#include "dsf_commands.h"

int RedisModule_OnLoad(RedisModuleCtx* ctx)
{
	if(REDISMODULE_ERR == RedisModule_Init(ctx, DSF_MODULE_NAME, DSF_MODULE_VERSION, REDISMODULE_APIVER_1))
		return REDISMODULE_ERR;

	if(REDISMODULE_ERR == DsfDataType_Register(ctx))
		return REDISMODULE_ERR;

	if(REDISMODULE_ERR == DsfCommands_Register(ctx))
		return REDISMODULE_ERR;

	return REDISMODULE_OK;
}
