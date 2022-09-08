#ifndef NETCDF4_ATTRIBUTE_H
#define NETCDF4_ATTRIBUTE_H

#include <napi.h>
#include <string>

namespace netcdf4async {

class Attribute {
    public:
    Attribute() {}
    ~Attribute() {}
   // static Napi::Promise::Deferred set_value(const Napi::Env env, int parent_id,int var_id,const std::string attribute_name,int type,const Napi::Value &value);
    static Napi::Promise::Deferred get_attributes(Napi::Env env, int parent_id, int var_id, bool return_type);
    // static Napi::Promise::Deferred rename(const Napi::Env env, int parent_id, int var_id, std::string old_name, std::string new_name);
    // static Napi::Promise::Deferred demove(const Napi::Env env, int parent_id, int var_id, std::string name);
};

}

#endif
