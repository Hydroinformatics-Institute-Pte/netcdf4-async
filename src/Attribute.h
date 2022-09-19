#ifndef NETCDF4_ATTRIBUTE_H
#define NETCDF4_ATTRIBUTE_H

#include <napi.h>
#include <string>
#include <node_version.h>

namespace netcdf4async {

#ifdef _MSC_VER
extern "C" {
#endif	

Napi::Value add_attribute(Napi::Env env, Napi::Promise::Deferred deferred, int parent_id, int var_id,
    const std::string attribute_name, int type, const Napi::Value &value);
Napi::Promise::Deferred get_attributes(Napi::Env env, int parent_id, int var_id, bool return_type);
Napi::Promise::Deferred rename_attribute(Napi::Env env, Napi::Promise::Deferred deferred, int parent_id, int var_id, std::string old_name, std::string new_name);
Napi::Promise::Deferred delete_attribute(Napi::Env env, Napi::Promise::Deferred deferred, int parent_id, int var_id, std::string name);
#ifdef _MSC_VER
}
#endif	


}

#endif
