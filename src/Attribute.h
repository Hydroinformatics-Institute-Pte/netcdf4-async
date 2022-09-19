#ifndef NETCDF4_ATTRIBUTE_H
#define NETCDF4_ATTRIBUTE_H

#include <napi.h>
#include <string>
#include <node_version.h>

namespace netcdf4async {


Napi::Value add_attribute(Napi::Env env, Napi::Promise::Deferred deferred, int parent_id, int var_id,
    const std::string attribute_name, int type, const Napi::Value &value);
Napi::Value get_attributes(Napi::Env env, int parent_id, int var_id, bool return_type);
Napi::Value rename_attribute(Napi::Env env, Napi::Promise::Deferred deferred, int parent_id,
    int var_id, const std::string old_name, const std::string new_name);
Napi::Value delete_attribute(Napi::Env env, Napi::Promise::Deferred deferred,
 int parent_id, int var_id, const std::string name);

}

#endif
