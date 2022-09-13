#ifndef NETCDF4_ATTRIBUTE_H
#define NETCDF4_ATTRIBUTE_H

#include <napi.h>
#include <string>
#include <node_version.h>

#define ATTR_TO_VAL(v,type,construct,array)                \
        if (nc_attribute->len == 1) {                      \
            value = Napi::construct::New(                  \
                env,                                       \
                nc_attribute->value.v[0]                   \
            );                                             \
        }                                                  \
        else {                                             \
            value = Napi::array::New(                      \
                env,                                       \
                sizeof(type),                              \
                Napi::ArrayBuffer::New(                    \
                    env,                                   \
                    nc_attribute->value.v,                 \
                    nc_attribute->len * sizeof(type)    \
                ),                                         \
                0,                                         \
                napi_int8_array                            \
            );                                             \
        }                                                  \
        delete[] nc_attribute->value.v;


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

#if NODE_MAJOR_VERSION > 8
#define BIGINT_TO_ATTR(type)   \
   else if (value.IsBigInt()) {                                        \
		attribute_value.len = 1;                                         \
		type* pv = new type[attribute_value.len];    \
        bool loseless=0;                    \
        Napi::Number number=Napi::Number::New(env,value.As<Napi::BigInt>().NAPI_Big(type)(&loseless)); \
		pv[0] = number.As<Napi::NAPI_Type(type)>().NAPI_Value(type)(); \
		attribute_value.value.v = pv;                                    \
    }

#else
#define BIGINT_TO_ATTR(type)
#endif


#define VAL_TO_ATTR(type)        \
    if (value.IsNumber()) {                                              \
		attribute_value.len = 1;                                         \
		type* pv = new type[attribute_value.len];                        \
		pv[0] = value.As<Napi::NAPI_Type(type)>().NAPI_Value(type)();    \
		attribute_value.value.v = pv;                                    \
    } BIGINT_TO_ATTR(type)                                                                   \
    else if (value.IsTypedArray()){                                      \
        auto array = value.As<Napi::NAPI_Array(type)>();                   \
        attribute_value.len = array.ElementLength();                     \
        attribute_value.value.v = array.ArrayBuffer().Data();            \
    } else {                                                             \
        deferred.Reject(                                                 \
            Napi::String::New(                                           \
                env,                                                     \
                "NetCDF4: Need either value or typed array"              \
            )                                                            \
        );                                                               \
        return deferred;                                                 \
    }


namespace netcdf4async {

Napi::Promise::Deferred add_attribute(Napi::Promise::Deferred deferred, Napi::Env env, int parent_id, int var_id,
    const std::string attribute_name, int type, const Napi::Value &value);
Napi::Promise::Deferred get_attributes(Napi::Env env, int parent_id, int var_id, bool return_type);
Napi::Promise::Deferred rename_attribute(Napi::Env env, int parent_id, int var_id, std::string old_name, std::string new_name);
Napi::Promise::Deferred delete_attribute(Napi::Env env, int parent_id, int var_id, std::string name);


}

#endif
