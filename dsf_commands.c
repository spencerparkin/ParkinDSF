#include "dsf_commands.h"
#include "dsf.h"

int DsfCommands_Register(RedisModuleCtx* ctx)
{
    if(REDISMODULE_ERR == RedisModule_CreateCommand(ctx, "DSFADD", DsfCommand_Add, "write fast", 1, 1, 1))
		return REDISMODULE_ERR;

    if(REDISMODULE_ERR == RedisModule_CreateCommand(ctx, "DSFREMOVE", DsfCommand_Remove, "write", 1, 1, 1))
        return REDISMODULE_ERR;

    // Due to path compression, technically we both read and write with this command, but Redis is not affected by the writing at all.
    if(REDISMODULE_ERR == RedisModule_CreateCommand(ctx, "DSFARECOMEMBERS", DsfCommand_AreComembers, "read fast", 1, 1, 1))
        return REDISMODULE_ERR;

    if(REDISMODULE_ERR == RedisModule_CreateCommand(ctx, "DSFISMEMBER", DsfCommand_IsMember, "read fast", 1, 1, 1))
        return REDISMODULE_ERR;

    // We also read-write here, but Redis is none-the-wiser of it.
    if(REDISMODULE_ERR == RedisModule_CreateCommand(ctx, "DSFUNION", DsfCommand_Union, "read fast", 1, 1, 1))
        return REDISMODULE_ERR;

    if(REDISMODULE_ERR == RedisModule_CreateCommand(ctx, "DSFCARD", DsfCommand_Cardinality, "read fast", 1, 1, 1))
        return REDISMODULE_ERR;

    if(REDISMODULE_ERR == RedisModule_CreateCommand(ctx, "DSFSIZE", DsfCommand_Size, "read fast", 1, 1, 1))
        return REDISMODULE_ERR;

    if(REDISMODULE_ERR == RedisModule_CreateCommand(ctx, "DSFFINDSET", DsfCommand_FindSet, "read", 1, 1, 1))
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}

static int DsfCommand_FindDsfDataType(RedisModuleCtx* ctx, RedisModuleString* keyName, RedisModuleKey** key, DsfData** dsf, int canCreate, int mode)
{
    *dsf = NULL;

    *key = RedisModule_OpenKey(ctx, keyName, mode);
    if(!*key)
        return RedisModule_ReplyWithError(ctx, "ERR Failed to open key");   // TODO: Is there a formatted reply API call?  Can I just make one?
    else
    {
        int keyType = RedisModule_KeyType(*key);
        if(keyType == REDISMODULE_KEYTYPE_EMPTY)
        {
            if(!canCreate)
                return RedisModule_ReplyWithError(ctx, "ERR key does not exist");
            else
            {
                *dsf = DsfDataType_Create();
                RedisModule_ModuleTypeSetValue(*key, dsfDataType, *dsf);
            }
        }
        else if(RedisModule_ModuleTypeGetType(*key) != dsfDataType)
            return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        else
            *dsf = RedisModule_ModuleTypeGetValue(*key);
    }

    // Note that we intentially leave the key open, because the caller
    // may want to use the opened key.  Also, the caller need not close
    // the key either, because we are using auto-memory management.
    return REDISMODULE_OK;
}

int DsfCommand_Add(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
{
    if(argc < 3)
        return RedisModule_WrongArity(ctx);

    DsfData* dsf = NULL;
    RedisModuleKey* key = NULL;
    if(REDISMODULE_ERR == DsfCommand_FindDsfDataType(ctx, argv[1], &key, &dsf, 1, REDISMODULE_WRITE))
        return REDISMODULE_ERR;

    for(uint32_t i = 2; i < argc; i++)
        if(REDISMODULE_ERR == DsfDataType_AddMember(dsf, argv[i], ctx))
            return REDISMODULE_ERR;

    RedisModule_CloseKey(key);

    return REDISMODULE_OK;
}

int DsfCommand_Remove(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
{
    return REDISMODULE_OK;
}

int DsfCommand_AreComembers(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
{
    if(argc != 4)
        return RedisModule_WrongArity(ctx);

    DsfData* dsf = NULL;
    RedisModuleKey* key = NULL;
    if(REDISMODULE_ERR == DsfCommand_FindDsfDataType(ctx, argv[1], &key, &dsf, 0, REDISMODULE_READ))
        return REDISMODULE_ERR;

    int result = 0;
    if(REDISMODULE_ERR == DsfDataType_AreComembers(dsf, argv[2], argv[3], &result, ctx))
        return REDISMODULE_ERR;

    return RedisModule_ReplyWithLongLong(ctx, result);
}

int DsfCommand_IsMember(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
{
    if(argc != 3)
        return RedisModule_WrongArity(ctx);

    DsfData* dsf = NULL;
    RedisModuleKey* key = NULL;
    if(REDISMODULE_ERR == DsfCommand_FindDsfDataType(ctx, argv[1], &key, &dsf, 0, REDISMODULE_READ))
        return REDISMODULE_ERR;
    
    int result = 0;
    if(REDISMODULE_ERR == DsfDataType_IsMember(dsf, argv[2], &result, ctx))
        return REDISMODULE_ERR;

    return RedisModule_ReplyWithLongLong(ctx, result);
}

int DsfCommand_Union(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
{
    if(argc != 4)
        return RedisModule_WrongArity(ctx);

    DsfData* dsf = NULL;
    RedisModuleKey* key = NULL;
    if(REDISMODULE_ERR == DsfCommand_FindDsfDataType(ctx, argv[1], &key, &dsf, 0, REDISMODULE_READ))
        return REDISMODULE_ERR;

    if(REDISMODULE_ERR == DsfDataType_Union(dsf, argv[2], argv[3], ctx))
        return REDISMODULE_ERR;

    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}

int DsfCommand_Cardinality(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
{
    if(argc != 2)
        return RedisModule_WrongArity(ctx);

    DsfData* dsf = NULL;
    RedisModuleKey* key = NULL;
    if(REDISMODULE_ERR == DsfCommand_FindDsfDataType(ctx, argv[1], &key, &dsf, 0, REDISMODULE_READ))
        return REDISMODULE_ERR;

    return RedisModule_ReplyWithLongLong(ctx, dsf->card);
}

int DsfCommand_Size(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
{
    if(argc != 2)
        return RedisModule_WrongArity(ctx);

    DsfData* dsf = NULL;
    RedisModuleKey* key = NULL;
    if(REDISMODULE_ERR == DsfCommand_FindDsfDataType(ctx, argv[1], &key, &dsf, 0, REDISMODULE_READ))
        return REDISMODULE_ERR;

    uint64_t size = RedisModule_DictSize(dsf->dict);
    return RedisModule_ReplyWithLongLong(ctx, size);
}

int DsfCommand_FindSet(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
{
    // TODO: Reply with array here.  If we're RESP3, reply with set?
    return REDISMODULE_OK;
}