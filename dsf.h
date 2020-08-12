#pragma once

#include <redismodule.h>

#define DSF_ENCODING_VERSION		1
#define DSF_TYPE_METHOD_VERSION		1

typedef struct _DsfData
{
	RedisModuleDict* dict;
	uint32_t card;
} DsfData;

typedef struct _DsfElement
{
	struct _DsfElement* rep;
	uint32_t rank;
} DsfElement;

extern RedisModuleType* dsfDataType;

int DsfDataType_Register(RedisModuleCtx* ctx);
void* DsfDataType_RdbLoad(RedisModuleIO* rdb, int encver);
void DsfDataType_RdbSave(RedisModuleIO* rdb, void* value);
void DsfDataType_AofRewrite(RedisModuleIO* aof, RedisModuleString* key, void* value);
size_t DsfDataType_MemUsage(void* value);
void DsfDataType_Digest(RedisModuleDigest* digest, void* value);
void DsfDataType_Free(void* value);
DsfData* DsfDataType_Create(void);
void DsfDataType_FreeElement(DsfElement* element);
DsfElement* DsfDataType_CreateElement(void);
int DsfDataType_AddMember(DsfData* dsf, RedisModuleString* member, RedisModuleCtx* ctx);
int DsfDataType_RemoveMember(DsfData* dsf, RedisModuleString* member, RedisModuleCtx* ctx);
int DsfDataType_AreComembers(DsfData* dsf, RedisModuleString* memberA, RedisModuleString* memberB, int* result, RedisModuleCtx* ctx);
int DsfDataType_IsMember(DsfData* dsf, RedisModuleString* member, int* result, RedisModuleCtx* ctx);
int DsfDataType_Union(DsfData* dsf, RedisModuleString* memberA, RedisModuleString* memberB, RedisModuleCtx* ctx);
int DsfDataType_FindSet(DsfData* dsf, RedisModuleString* member, RedisModuleDict** set, RedisModuleCtx* ctx);
int DsfDataType_CreateDump(DsfData* dsf, RedisModuleDict*** dump, RedisModuleCtx* ctx);
void DsfDataType_FreeDump(DsfData* dsf, RedisModuleDict** dump, RedisModuleCtx* ctx);