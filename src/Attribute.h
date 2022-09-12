#ifndef NETCDF4_ATTRIBUTE_H
#define NETCDF4_ATTRIBUTE_H

#include <napi.h>
#include <string>

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
                    nc_attribute->len * sizeof(int16_t)    \
                ),                                         \
                0,                                         \
                napi_int8_array                            \
            );                                             \
        }                                                  \
        delete[] nc_attribute->value.v;

#define VAL_TO_ATTR(type, napi_type,construct)                           \
    if (value.IsNumber()) {                                              \
		attribute_value.len = 1;                                         \
		type* pv = new type[attribute_value.len];                        \
		pv[0] = value.As<Napi::napi_type>().construct();			     \
		attribute_value.value.v = pv;                                    \
    }

#define VAL_TO_BIG_ATTR(type, napi_type,construct)                       \
    if (value.IsNumber()) {                                              \
		attribute_value.len = 1;                                         \
		type* pv = new type[attribute_value.len];                        \
		pv[0] = value.As<Napi::napi_type>().construct(nullptr);			 \
		attribute_value.value.v = pv;                                    \
    }


namespace netcdf4async {

Napi::Promise::Deferred add_attribute(Napi::Promise::Deferred deferred, Napi::Env env, int parent_id, int var_id,
    const std::string attribute_name, int type, const Napi::Value &value);
Napi::Promise::Deferred get_attributes(Napi::Env env, int parent_id, int var_id, bool return_type);
Napi::Promise::Deferred rename_attribute(Napi::Env env, int parent_id, int var_id, std::string old_name, std::string new_name);
Napi::Promise::Deferred delete_attribute(Napi::Env env, int parent_id, int var_id, std::string name);


}

#endif
