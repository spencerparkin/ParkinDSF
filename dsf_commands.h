#pragma once

#include <redismodule.h>

int DsfCommands_Register(RedisModuleCtx* ctx);
int DsfCommand_Add(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int DsfCommand_Remove(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int DsfCommand_AreComembers(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int DsfCommand_IsMember(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int DsfCommand_Union(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int DsfCommand_Cardinality(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int DsfCommand_Size(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int DsfCommand_FindSet(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);