#include "dsf.h"
#include <stdlib.h>

RedisModuleType* dsfDataType = NULL;

static DsfElement* DsfElement_FindSetRep(DsfElement* element)
{
	DsfElement* rep = element;

	while(rep->rep)
		rep = rep->rep;

	// The following code is not required for correctness.
	// It is merely an optimization.  Point all elements up
	// the chain directly to the set representative.  Notice
	// that this does not change the time-complexity of the
	// routine.
	while(element->rep)
	{
		DsfElement* next = element->rep;
		element->rep = rep;
		element = next;
	}

	return rep;
}

static int DsfElement_SameSet(DsfElement* elementA, DsfElement* elementB)
{
	return DsfElement_FindSetRep(elementA) == DsfElement_FindSetRep(elementB);
}

static int DsfDataType_ReplyErrorAboutMember(RedisModuleCtx* ctx, const char* fmt, RedisModuleString* member)
{
	const char* memberCStr = RedisModule_StringPtrLen(member, NULL);
	RedisModuleString* errorMsg = RedisModule_CreateStringPrintf(ctx, fmt, memberCStr);
	const char* errorMsgCStr = RedisModule_StringPtrLen(errorMsg, NULL);
	RedisModule_ReplyWithError(ctx, errorMsgCStr);
	return REDISMODULE_ERR;
}

static int DsfDataType_ReplyErrorAboutMembers(RedisModuleCtx* ctx, const char* fmt, RedisModuleString* memberA, RedisModuleString* memberB)
{
	const char* memberACStr = RedisModule_StringPtrLen(memberA, NULL);
	const char* memberBCStr = RedisModule_StringPtrLen(memberB, NULL);
	RedisModuleString* errorMsg = RedisModule_CreateStringPrintf(ctx, fmt, memberACStr, memberBCStr);
	const char* errorMsgCStr = RedisModule_StringPtrLen(errorMsg, NULL);
	RedisModule_ReplyWithError(ctx, errorMsgCStr);
	return REDISMODULE_ERR;
}

int DsfDataType_Register(RedisModuleCtx* ctx)
{
	if(dsfDataType != NULL)
		return REDISMODULE_ERR;

	RedisModuleTypeMethods dsfDataTypeMethods =
	{
		DSF_TYPE_METHOD_VERSION,
		DsfDataType_RdbLoad,
		DsfDataType_RdbSave,
		DsfDataType_Rewrite,
		NULL,
		NULL,
		DsfDataType_Free,
		NULL,
		NULL,
		0
	};

	dsfDataType = RedisModule_CreateDataType(ctx, "parkinDSF", DSF_ENCODING_VERSION, &dsfDataTypeMethods);
	if(dsfDataType == NULL)
		return REDISMODULE_ERR;

	return REDISMODULE_OK;
}

void* DsfDataType_RdbLoad(RedisModuleIO* rdb, int encver)
{
	return NULL;
}

void DsfDataType_RdbSave(RedisModuleIO* rdb, void* value)
{
}

void DsfDataType_Rewrite(RedisModuleIO* aof, RedisModuleString* key, void* value)
{
}

size_t DsfDataType_MemUsage(void* value)
{
	return 0;
}

void DsfDataType_Digest(RedisModuleDigest* digest, void* value)
{
}

void DsfDataType_Free(void* value)
{
	DsfData* dsf = (DsfData*)value;

	RedisModuleDictIter* iter = RedisModule_DictIteratorStart(dsf->dict, "", NULL);
	for(;;)
	{
		DsfElement* element = NULL;
		void* key = (DsfElement*)RedisModule_DictNextC(iter, NULL, (void**)&element);
		if(key == NULL)
			break;
		
		DsfDataType_FreeElement(element);
	}

	RedisModule_DictIteratorStop(iter);
	RedisModule_FreeDict(NULL, dsf->dict);
	RedisModule_Free(dsf);
}

DsfData* DsfDataType_Create(void)
{
	DsfData* dsf = RedisModule_Alloc(sizeof(DsfData));
	dsf->dict = RedisModule_CreateDict(NULL);
	dsf->card = 0;
	return dsf;
}

void DsfDataType_FreeElement(DsfElement* element)
{
	RedisModule_Free(element);
}

DsfElement* DsfDataType_CreateElement(void)
{
	DsfElement* element = RedisModule_Alloc(sizeof(DsfElement));
	element->rep = NULL;
	element->rank = 1;
	return element;
}

int DsfDataType_AddMember(DsfData* dsf, RedisModuleString* member, RedisModuleCtx* ctx)
{
	DsfElement* element = RedisModule_DictGet(dsf->dict, member, NULL);
	if(element != NULL)
		return DsfDataType_ReplyErrorAboutMember(ctx, "ERR %s already exists", member);

	element = DsfDataType_CreateElement();
	RedisModule_DictSet(dsf->dict, member, element);

	dsf->card++;

	return REDISMODULE_OK;
}

int DsfDataType_RemoveMember(DsfData* dsf, RedisModuleString* member, RedisModuleCtx* ctx)
{
	return 0;
}

int DsfDataType_AreComembers(DsfData* dsf, RedisModuleString* memberA, RedisModuleString* memberB, int* result, RedisModuleCtx* ctx)
{
	DsfElement* elementA = RedisModule_DictGet(dsf->dict, memberA, NULL);
	if(elementA == NULL)
		return DsfDataType_ReplyErrorAboutMember(ctx, "ERR %s does not exist", memberA);

	DsfElement* elementB = RedisModule_DictGet(dsf->dict, memberB, NULL);
	if(elementB == NULL)
		return DsfDataType_ReplyErrorAboutMember(ctx, "ERR %s does not exist", memberB);

	*result = DsfElement_SameSet(elementA, elementB);

	return REDISMODULE_OK;
}

int DsfDataType_IsMember(DsfData* dsf, RedisModuleString* member, int* result, RedisModuleCtx* ctx)
{
	DsfElement* element = RedisModule_DictGet(dsf->dict, member, NULL);

	*result = (element != NULL);

	return REDISMODULE_OK;
}

int DsfDataType_Union(DsfData* dsf, RedisModuleString* memberA, RedisModuleString* memberB, RedisModuleCtx* ctx)
{
	DsfElement* elementA = RedisModule_DictGet(dsf->dict, memberA, NULL);
	if(elementA == NULL)
		return DsfDataType_ReplyErrorAboutMember(ctx, "ERR %s does not exist", memberA);

	DsfElement* elementB = RedisModule_DictGet(dsf->dict, memberB, NULL);
	if(elementB == NULL)
		return DsfDataType_ReplyErrorAboutMember(ctx, "ERR %s does not exist", memberB);

	elementA = DsfElement_FindSetRep(elementA);
	elementB = DsfElement_FindSetRep(elementB);
	if(elementA == elementB)
		return DsfDataType_ReplyErrorAboutMembers(ctx, "ERR %s and %s already members of the same set", memberA, memberB);

	// For correctness, it doesn't matter which representative
	// becomes the representative of the union of the two sets.
	// However, if we choose which one based upon an upper-bound
	// on the maximum length of a chain, then we can keep the
	// trees well balanced.
	if(elementA->rank < elementB->rank)
		elementA->rep = elementB;
	else if(elementB->rank < elementA->rank)
		elementB->rep = elementA;
	else
	{
		// Here the choice is completely arbitrary, but we
		// must bump rank.  Choose randomly.
		if(rand() > RAND_MAX / 2)
		{
			elementA->rep = elementB;
			elementB->rank++;
		}
		else
		{
			elementB->rep = elementA;
			elementA->rank++;
		}
	}

	dsf->card--;

	return REDISMODULE_OK;
}

