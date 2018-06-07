#include "lwm2m_objects.hpp"
#include <cstring>

int lwm2m_object_prepare_full_response(lwm2m_data_t ** dataArrayP, Lwm2mObjectBase* metaP)
{
    *dataArrayP = lwm2m_data_new(metaP->resources_len());
    if (*dataArrayP == NULL) return -1;
    int readable = 0;
    for(unsigned i=0;i<metaP->resources_len();++i)
    {
        lwm2m_object_res_item_t* resP = metaP->resource(i);
        if (!(resP->access & O_RES_R))
            continue;
        lwm2m_data_t* dataP = &(*dataArrayP)[readable];
        dataP->id = resP->ressource_id;
        ++readable;
    }
    return readable;
}

uint8_t lwm2m_object_assign_single_value(lwm2m_data_t* destination, lwm2m_object_res_item_t* resP, void* instanceP)
{
    lwm2m_object_util_type_t res_type = (lwm2m_object_util_type_t)resP->type;

    void* memberP = (void*)((char*)instanceP + resP->struct_member_offset);

    /// Support for function results as value ///
    if ((resP->access & O_RES_E) == O_RES_E)
    {
        if ((resP->access & O_RES_RW)==O_RES_RW){
            struct ReadWriteStruct
            {
                void*(*read)();
                void(*write)(void*);
            };
            ReadWriteStruct *cbStr =(ReadWriteStruct*) memberP;
            // We do not know the result type of f() yet and use a generic "void*"
            void* fdata = cbStr->read();
            // Make memberP point to the address of fData.
            // The dereferenced memberP will contain the real data.
            memberP = &fdata;
        } else if ((resP->access & O_RES_R)==O_RES_R){
            // The following is equal to this: readFunc f = (readFunc)(*(void**)memberP);
            // But the -pedantic switch forces us to use this union construct.
            union FunUnion{readFunc to; void *from;};
            readFunc f = ((FunUnion*)memberP)->to;
            // We do not know the result type of f() yet and use a generic "void*"
            void* fdata = f();
            // Make memberP point to the address of fData.
            // The dereferenced memberP will contain the real data.
            memberP = &fdata;
        } else
            return COAP_405_METHOD_NOT_ALLOWED;
    }
    ///////////////////////////////////////////////////////////////////////

    switch (res_type)
    {
    case O_RES_STRING:
        lwm2m_data_encode_string(*((char**)memberP), destination);
        return COAP_205_CONTENT;
    case O_RES_STRING_PREALLOC: {
        OpaqueType_t* o = (OpaqueType_t*)memberP;
        lwm2m_data_encode_string((char*)o->data, destination);
        return COAP_205_CONTENT;
    }
    case O_RES_OPAQUE_INDIRECT: {
        OpaqueIndirect* o = (OpaqueIndirect*)memberP;
        lwm2m_data_encode_opaque(o->data,o->used_len, destination);
        return COAP_205_CONTENT;
    }
    case O_RES_OPAQUE_PREALLOC: {
        OpaqueType_t* o = (OpaqueType_t*)memberP;
        lwm2m_data_encode_opaque(o->data,o->used_len, destination);
        return COAP_205_CONTENT;
    }
    case O_RES_BOOL:
        lwm2m_data_encode_bool(*((bool*)memberP), destination);
        return COAP_205_CONTENT;
    case O_RES_DOUBLE:
        lwm2m_data_encode_float(*((double*)memberP), destination);
        return COAP_205_CONTENT;
    case O_RES_INT8:
    case O_RES_UINT8:
        lwm2m_data_encode_int(*((int8_t*)memberP), destination);
        return COAP_205_CONTENT;
    case O_RES_INT16:
    case O_RES_UINT16:
        lwm2m_data_encode_int(*((int16_t*)memberP), destination);
        return COAP_205_CONTENT;
    case O_RES_INT32:
    case O_RES_UINT32:
        lwm2m_data_encode_int(*((int32_t*)memberP), destination);
        return COAP_205_CONTENT;
    case O_RES_INT64:
        lwm2m_data_encode_int(*((int64_t*)memberP), destination);
        return COAP_205_CONTENT;
    case O_RES_EXEC:
        break;
    }

    return COAP_404_NOT_FOUND;
}

static uint8_t prv_read(uint16_t instanceId,
                        int * numDataP,
                        lwm2m_data_t ** dataArrayP,
                        lwm2m_object_t * objectP)
{
    lwm2m_list_t* instanceP = (lwm2m_list_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == instanceP) return COAP_404_NOT_FOUND;

    Lwm2mObjectBase* metaP = Lwm2mObjectBase::getBase(objectP);
    if (!metaP) return COAP_500_INTERNAL_SERVER_ERROR;

    if (*numDataP == 0)
        *numDataP = lwm2m_object_prepare_full_response(dataArrayP, metaP);

    if (*numDataP < 0)
        return COAP_500_INTERNAL_SERVER_ERROR;

    for (int i = 0 ; i < *numDataP ; i++)
    {
        lwm2m_data_t* dataP = &(*dataArrayP)[i];
        unsigned res_id = dataP->id;
        lwm2m_object_res_item_t* resP = metaP->find_ressource(res_id);
        if (!resP)
            return COAP_404_NOT_FOUND;

        if (!(resP->access & O_RES_R) || resP->struct_member_offset==0)
            return COAP_405_METHOD_NOT_ALLOWED;

        lwm2m_object_assign_single_value(dataP, resP, instanceP);
    }

    return COAP_205_CONTENT;
}

/// Support for function results as value ///
template<class T>
static uint8_t writeToMemberOrToMethod(uint8_t access, void* memberP, T value) {
    if ((access & O_RES_E) == O_RES_E)
    {
        if ((access & O_RES_RW)==O_RES_RW){
            IndirectReadWrite<T> *cbStr =reinterpret_cast<IndirectReadWrite<T>*>(memberP);
            cbStr->write(value);
        } else if ((access & O_RES_W)==O_RES_W){
            IndirectWrite<T> f = *reinterpret_cast<IndirectWrite<T>*>(memberP);
            f(value);
        } else
            return COAP_405_METHOD_NOT_ALLOWED;
    } else {
        *((T*)memberP) = (T)value;
    }
    return COAP_204_CHANGED;
}

static uint8_t prv_write(uint16_t instanceId,
                         int numData,
                         lwm2m_data_t * dataArray,
                         lwm2m_object_t * objectP)
{
    lwm2m_list_t* instanceP = (lwm2m_list_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == instanceP) return COAP_404_NOT_FOUND;

    Lwm2mObjectBase* metaP = Lwm2mObjectBase::getBase(objectP);

    // For the verify/write callback to be able to deny a change, we hand over a copy
    // of the instance data. The copy is just thrown away if the change is denied.
    uint8_t copy_of_entry[300];
    if (metaP->object_instance_size>300) return COAP_412_PRECONDITION_FAILED;

    for (int i = 0 ; i < numData ; ++i)
    {
        unsigned res_id = dataArray[i].id;

        lwm2m_object_res_item_t* resP = metaP->find_ressource(res_id);
        if (!resP)
            return COAP_404_NOT_FOUND;

        // Check for write access and if offset is set
        if (!(resP->access & O_RES_W))
            return COAP_405_METHOD_NOT_ALLOWED;

        // Extract ressource type and member pointer from meta object
        lwm2m_object_util_type_t res_type = (lwm2m_object_util_type_t)resP->type;

        // For the verify/write callback to be able to deny a change, we hand over a copy
        // of the instance data. The copy is just thrown away if the change is denied.
        memcpy(copy_of_entry, instanceP, metaP->object_instance_size);
        void* memberP = (void*)((char*)copy_of_entry + resP->struct_member_offset);

        union {
            bool b;
            int64_t i;
            double d;
            char* c;
        } temp;

        // The CPP-API does not reallocate memory.
        // If there's not enough space, a COAP_413_ENTITY_TOO_LARGE error code is returned.

        switch (res_type)
        {
        case O_RES_OPAQUE_INDIRECT: {
            // We do not apply data directly to the instance object. To make this work for
            // O_RES_OPAQUE_INDIRECT, we let the data pointer point
            // to the receive buffer temporary and copy the data in a post-processing step after
            // the verify/write callback accepted the change.
            OpaqueIndirect* o = (OpaqueIndirect*)memberP;
            if (o->reserved_len < dataArray[i].value.asBuffer.length)
                return COAP_413_ENTITY_TOO_LARGE;
            temp.c = (char*)o->data; // backup orginal pointer for post-processing
            o->used_len = dataArray[i].value.asBuffer.length;
            o->data = dataArray[i].value.asBuffer.buffer;
            break;
        }
        case O_RES_OPAQUE_PREALLOC:
        case O_RES_STRING_PREALLOC: {
            OpaqueType_t* o = (OpaqueType_t*)memberP;
            if (o->reserved_len < dataArray[i].value.asBuffer.length)
                return COAP_413_ENTITY_TOO_LARGE;
            o->used_len = dataArray[i].value.asBuffer.length;
            memcpy(o->data, (char*)dataArray[i].value.asBuffer.buffer, o->used_len);
            break;
        }
        case O_RES_BOOL:
            if (1 != lwm2m_data_decode_bool(&dataArray[i], &temp.b))
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod(resP->access,memberP,temp.b);
            break;
        case O_RES_DOUBLE:
            if (1 != lwm2m_data_decode_float(&dataArray[i], &temp.d))
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod(resP->access,memberP,temp.d);
            break;
        case O_RES_INT8:
            if (1 != lwm2m_data_decode_int(&dataArray[i], &temp.i) || temp.i < INT8_MIN || temp.i > INT8_MAX)
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod<int8_t>(resP->access,memberP, temp.i);
            break;
        case O_RES_UINT8:
            if (1 != lwm2m_data_decode_int(&dataArray[i], &temp.i) || temp.i < 0 || temp.i > UINT8_MAX)
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod<uint8_t>(resP->access,memberP,temp.i);
            break;
        case O_RES_INT16:
            if (1 != lwm2m_data_decode_int(&dataArray[i], &temp.i) || temp.i < INT16_MIN || temp.i > INT16_MAX)
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod<int16_t>(resP->access,memberP,temp.i);
            break;
        case O_RES_UINT16:
            if (1 != lwm2m_data_decode_int(&dataArray[i], &temp.i) || temp.i < 0 || temp.i > UINT16_MAX)
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod<uint16_t>(resP->access,memberP,temp.i);
            break;
        case O_RES_INT32:
            if (1 != lwm2m_data_decode_int(&dataArray[i], &temp.i) || temp.i < INT32_MIN || temp.i > INT32_MAX)
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod<int32_t>(resP->access,memberP,temp.i);
            break;
        case O_RES_UINT32:
            if (1 != lwm2m_data_decode_int(&dataArray[i], &temp.i) || temp.i < 0 || temp.i > UINT32_MAX)
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod<uint32_t>(resP->access,memberP,temp.i);
            break;
        case O_RES_INT64:
            if (1 != lwm2m_data_decode_int(&dataArray[i], &temp.i))
                return COAP_400_BAD_REQUEST;
            writeToMemberOrToMethod<int64_t>(resP->access,memberP,temp.i);
            break;
        case O_RES_STRING:
            if (!(resP->access & O_RES_E))
                return COAP_400_BAD_REQUEST;
            else {
                temp.c = (char*)dataArray[i].value.asBuffer.buffer;
                writeToMemberOrToMethod<const char*>(resP->access,memberP,temp.c);
                break;
            }
        default:
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        if (metaP->verifyWrite((Lwm2mObjectInstance*)copy_of_entry, res_id))
        {
            // The O_RES_OPAQUE_INDIRECT needs some special handling:
            if (res_type == O_RES_OPAQUE_INDIRECT) {
                OpaqueIndirect* o = (OpaqueIndirect*)memberP;
                // Restore the pointer to the original memory area
                o->data = (uint8_t*)temp.c;
                // Copy the receive buffer to the destination memory address
                memcpy(o->data, (char*)dataArray[i].value.asBuffer.buffer, o->used_len);
            }
            // The verify method is either not set or accepted the input. Copy the altered
            // instance data back to the original instance.
            memcpy(instanceP, copy_of_entry, metaP->object_instance_size);
        } else {
            return COAP_400_BAD_REQUEST;
        }

        lwm2m_resource_changed_event(objectP->objID, instanceId, resP->ressource_id);
    }

    return COAP_204_CHANGED;
}


uint8_t prv_execute(uint16_t instanceId,
                    uint16_t res_id,
                    uint8_t * buffer,
                    int length,
                    lwm2m_object_t * objectP)
{
    Lwm2mObjectBase* metaP = Lwm2mObjectBase::getBase(objectP);

    lwm2m_list_t* instanceP = (lwm2m_list_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == instanceP) return COAP_404_NOT_FOUND;

    lwm2m_object_res_item_t* resP = metaP->find_ressource(res_id);
    if (!resP)
        return COAP_404_NOT_FOUND;

    if (resP->access != O_RES_E)
        return COAP_405_METHOD_NOT_ALLOWED;

    void* memberP = (void*)((char*)instanceP + resP->struct_member_offset);
    // The following is equal to this: execFunc f = (execFunc)(*(void**)memberP);
    // But the -pedantic switch forces us to use this union construct.
    union FunUnion{execFunc to; void *from;};
    execFunc f = ((FunUnion*)memberP)->to;
    if (f)
        f(buffer, length);
    return COAP_204_CHANGED;
}


static uint8_t prv_discover(uint16_t instanceId,
                            int * numDataP,
                            lwm2m_data_t ** dataArrayP,
                            lwm2m_object_t * objectP)
{
    Lwm2mObjectBase* metaP = Lwm2mObjectBase::getBase(objectP);
    if (!metaP) return COAP_500_INTERNAL_SERVER_ERROR;

    lwm2m_list_t* instanceP = (lwm2m_list_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == instanceP) return COAP_404_NOT_FOUND;

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        const unsigned resources_len = metaP->resources_len();
        *dataArrayP = lwm2m_data_new(resources_len);
        if (*dataArrayP == NULL)
            return COAP_500_INTERNAL_SERVER_ERROR;
        int readable = 0;
        for(unsigned i=0;i<resources_len;++i)
        {
            lwm2m_object_res_item_t* resP = metaP->resource(i);
            if (!(resP->access & (O_RES_RW|O_RES_E)))
                continue;
            lwm2m_data_t* dataP = &(*dataArrayP)[readable];
            dataP->id = resP->ressource_id;
            ++readable;
        }
        *numDataP = readable;
        return COAP_205_CONTENT;
    }

    for (int i = 0; i < *numDataP; ++i)
    {
        lwm2m_data_t* dataP = &(*dataArrayP)[i];
        if (dataP->id >= metaP->resources_len()) {
            return COAP_404_NOT_FOUND;
        }
    }
    return COAP_205_CONTENT;
}


static uint8_t prv_delete(uint16_t id, lwm2m_object_t * objectP)
{
    Lwm2mObjectBase* metaP = Lwm2mObjectBase::getBase(objectP);
    if (!metaP) return COAP_500_INTERNAL_SERVER_ERROR;

    lwm2m_list_t * targetP;
    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&targetP);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    metaP->deleteInstance(static_cast<Lwm2mObjectInstance*>(targetP));

    return COAP_202_DELETED;
}

static uint8_t prv_create(uint16_t instanceId,
                          int numData,
                          lwm2m_data_t * dataArray,
                          lwm2m_object_t * objectP)
{
    Lwm2mObjectBase* metaP = Lwm2mObjectBase::getBase(objectP);
    if (!metaP) return COAP_500_INTERNAL_SERVER_ERROR;

    Lwm2mObjectInstance* instanceP = metaP->createInstance(instanceId);
    if (NULL == instanceP)
        return COAP_500_INTERNAL_SERVER_ERROR;

    instanceP->id = instanceId;
    objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, instanceP);

    uint8_t result = prv_write(instanceId, numData, dataArray, objectP);

    if (result != COAP_204_CHANGED)
    {
        (void)prv_delete(instanceId, objectP);
    }
    else
    {
        result = COAP_201_CREATED;
    }

    return result;
}

void Lwm2mObjectBase::addInstance(lwm2m_context_t * contextP, Lwm2mObjectInstance* instance) {
    if (lwm2m_list_find(object.instanceList,instance->id)) return;

    object.instanceList=lwm2m_list_add(object.instanceList, instance);
    if (contextP->state == STATE_READY && LWM2M_LIST_FIND(contextP->objectList, object.objID) != NULL)
        lwm2m_update_registration(contextP, 0, true);
}
lwm2m_list_t* Lwm2mObjectBase::removeInstance(lwm2m_context_t * contextP, uint16_t instance_id) {
    lwm2m_list_t* removed = NULL;
    object.instanceList = lwm2m_list_remove(object.instanceList, instance_id, &removed);

    if (contextP->state == STATE_READY && LWM2M_LIST_FIND(contextP->objectList, object.objID) != NULL)
    {
        lwm2m_update_registration(contextP, 0, true);
    }
    return removed;
}

int Lwm2mObjectBase::registerObject(lwm2m_context_t * contextP, bool allow_dynamic_instance_creation) {
    object.readFunc = prv_read;
    object.writeFunc = prv_write;
    object.discoverFunc = prv_discover;
    object.executeFunc = prv_execute;

    if (allow_dynamic_instance_creation)
    {
        object.createFunc = prv_create;
        object.deleteFunc = prv_delete;
    }

    return lwm2m_add_object(contextP, &object);
}
