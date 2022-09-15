#ifndef MACROS_H
#define MACROS_H


#include <string.h>
#include <stdexcept>
#include <node_version.h>

#define NAPI_uint8_t Number
#define NAPI_uint16_t Number
#define NAPI_uint32_t Number
#define NAPI_int8_t Number
#define NAPI_int16_t Number
#define NAPI_int32_t Number
#define NAPI_double Number
#define NAPI_float Number
#define NAPI_uint64_t Number
#define NAPI_int64_t Number

#define NAPI_Array_uint8_t Uint8Array
#define NAPI_Array_uint16_t Uint16Array
#define NAPI_Array_uint32_t Uint32Array
#define NAPI_Array_int8_t Int8Array
#define NAPI_Array_int16_t Int16Array
#define NAPI_Array_int32_t Int32Array
#define NAPI_Array_double Float64Array
#define NAPI_Array_float Float32Array
#define NAPI_Array_int64_t BigInt64Array
#define NAPI_Array_uint64_t BigUint64Array

#define NAPI_Value_uint8_t Int32Value
#define NAPI_Value_uint16_t Int32Value
#define NAPI_Value_uint32_t Int32Value
#define NAPI_Value_int8_t Int32Value
#define NAPI_Value_int16_t Int32Value
#define NAPI_Value_int32_t Int32Value
#define NAPI_Value_double DoubleValue
#define NAPI_Value_float FloatValue
#define NAPI_Value_int64_t Int64Value
#define NAPI_Value_uint64_t Int64Value

#define NAPI_Attr_uint8_t Number
#define NAPI_Attr_uint16_t Number
#define NAPI_Attr_uint32_t Number
#define NAPI_Attr_int8_t Number
#define NAPI_Attr_int16_t Number
#define NAPI_Attr_int32_t Number
#define NAPI_Attr_double Number
#define NAPI_Attr_float Number
#define NAPI_Attr_uint64_t BigInt
#define NAPI_Attr_int64_t BigInt

#define NAPI_Union_uint8_t u8
#define NAPI_Union_uint16_t u16
#define NAPI_Union_uint32_t u32
#define NAPI_Union_int8_t i8
#define NAPI_Union_int16_t i16
#define NAPI_Union_int32_t i32
#define NAPI_Union_double d
#define NAPI_Union_float f
#define NAPI_Union_uint64_t u64
#define NAPI_Union_int64_t i64
#define NAPI_Union_char s


#define NAPI_Big_uint8_t Uint64Value
#define NAPI_Big_uint16_t Uint64Value
#define NAPI_Big_uint32_t Uint64Value
#define NAPI_Big_int8_t Int64Value
#define NAPI_Big_int16_t Int64Value
#define NAPI_Big_int32_t Int64Value
#define NAPI_Big_double Int64Value
#define NAPI_Big_float Int64Value
#define NAPI_Big_int64_t Int64Value
#define NAPI_Big_uint64_t Uint64Value

#define NAPI_Array(v) NAPI_Array_##v
#define NAPI_Value(v) NAPI_Value_##v
#define NAPI_Type(v) NAPI_##v
#define NAPI_Big(v) NAPI_Big_##v
#define NAPI_Attr(v) NAPI_Attr_##v
#define NAPI_Union(v) NAPI_Union_##v

#define TYPED_VALUE(name,type,size)  \
        name.NAPI_Union(type)=new type[size];

#define ITEM_TO_VAL(type)                                          \
        if (nc_item->len == 1) {                              \
            value = Napi::NAPI_Attr(type)::New(                    \
                env,                                               \
                nc_item->value.NAPI_Union(type)[0]            \
            );                                                     \
            delete[] nc_item->value.NAPI_Union(type);         \
        }                                                          \
        else {                                                     \
            /* Workaround bug in nodejs v8. */                     \
            /* At least after Node 14, confirmed in Node 16 */     \
            /* See https://github.com/nodejs/node/issues/32463 */  \
            void* data = malloc(nc_item->len * sizeof(type)); \
            memcpy(                                                \
                data,                                              \
                nc_item->value.v,                             \
                nc_item->len * sizeof(type)                   \
            );                                                     \
            auto ab=Napi::ArrayBuffer::New(                        \
                env,                                               \
                data,                                              \
                nc_item->len * sizeof(type),                  \
                [](Napi::Env env, void *data) {                    \
                        free(data);                                \
                }                                                  \
            );                                                     \
            value = Napi::NAPI_Array(type)::New(                   \
                env,                                               \
                nc_item->len,                                 \
                ab,                                                \
                0                                                  \
            );                                                     \
        }                                                          \



#if NODE_MAJOR_VERSION > 8
#define BIGINT_TO_ITEM(type)                                           \
   else if (value.IsBigInt()) {                                        \
		nc_item.len = 1;                                       \
		type* pv = new type[nc_item.len];                      \
        bool loseless=0;                                               \
        Napi::Number number=Napi::Number::New(                         \
            env,                                                       \
            value.As<Napi::BigInt>().NAPI_Big(type)(&loseless)         \
        );                                                             \
		pv[0] = number.As<Napi::NAPI_Type(type)>().NAPI_Value(type)(); \
		nc_item.value.v = pv;                                  \
    }

#else
#define BIGINT_TO_ITEM(type)
#endif


#define VAL_TO_ITEM(type)                                                \
    if (value.IsNumber()) {                                              \
		nc_item.len = 1;                                         \
		type* pv = new type[nc_item.len];                        \
		pv[0] = value.As<Napi::NAPI_Type(type)>().NAPI_Value(type)();    \
		nc_item.value.v = pv;                                    \
    } BIGINT_TO_ITEM(type)                                               \
    else if (value.IsTypedArray()){                                      \
        auto array = value.As<Napi::NAPI_Array(type)>();                 \
        nc_item.len = array.ElementLength();                     \
        nc_item.value.v = array.ArrayBuffer().Data();            \
    } else {                                                             \
        deferred.Reject(                                                 \
            Napi::String::New(                                           \
                env,                                                     \
                "NetCDF4: Need either value or typed array"              \
            )                                                            \
        );                                                               \
        return deferred.Promise();                                                 \
    }

namespace netcdf4async {

    union UnionType{
        int8_t* i8;
        int16_t* i16;
        int32_t* i32;
        float* f;
        double* d;
        uint8_t* u8;
        uint16_t* u16;
        uint32_t* u32;
        uint64_t* u64;
        int64_t* i64;
        char* s;
        char** ps;
        const char* text;
        void* v;
        size_t* size;
    };
    struct Item
    {
        std::string name;
        int type;
        size_t len;
        UnionType value;
    };

    Napi::Value item2value(Napi::Env env, Item *nc_item);
    void typedValue(Item* pItem);
}
#endif