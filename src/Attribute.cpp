#include "Attribute.h"
#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"
#include "utils.h"

namespace netcdf4async {
struct attr_struct
{
	std::string name;
	int type;
	size_t len;
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
	};
	UnionType value;
};


struct NCAttribute_list
{
	std::vector<attr_struct> attributes;
};

Napi::Promise::Deferred Attribute::get_attributes(Napi::Env env, int parent_id, int var_id, bool return_type) {
    auto worker = new NCAsyncWorker<NCAttribute_list>(
		env,
		[parent_id, var_id] (const NCAsyncWorker<NCAttribute_list>* worker) {
			int natts;
            if (var_id == NC_GLOBAL){
			    NC_CALL(nc_inq_natts(parent_id, &natts)); // group attribute
            } else { 
                NC_CALL(nc_inq_varnatts(parent_id, var_id, &natts)); //variable attribute
            }

			char name[NC_MAX_NAME + 1];
			NCAttribute_list result;
			
			for (int i = 0; i < natts; ++i) {
				int type;
				size_t len;
				attr_struct attribute;
				NC_CALL(nc_inq_attname(parent_id, var_id, i, name));
				NC_CALL(nc_inq_attlen(parent_id, var_id, name, &len));
				NC_CALL(nc_inq_atttype(parent_id, var_id, name, &type));
				
				switch (type) {
				case NC_BYTE: {
					attribute.value.i8 = new int8_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.i8));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				case NC_SHORT: {
					attribute.value.i16 = new int16_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.i16));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
				
				} break;
				case NC_INT: {
					attribute.value.i32 = new int32_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.i32));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				case NC_FLOAT: {
					attribute.value.f = new float[len];
					NC_CALL(nc_get_att(parent_id, var_id, name,attribute.value.f));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				case NC_DOUBLE: {
					attribute.value.d = new double[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.d));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				case NC_UBYTE: {
					attribute.value.u8 = new uint8_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.u8));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
				} break;
				case NC_USHORT: {
					attribute.value.u16 = new uint16_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.u16));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
				} break;
				case NC_UINT: {
					attribute.value.u32 = new uint32_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.u32));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
#if NODE_MAJOR_VERSION > 8
				case NC_UINT64: {
					attribute.value.u64 = new uint64_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name,attribute.value.u64));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
		
				} break;
				case NC_INT64: {
					attribute.value.i64 = new int64_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name,attribute.value.i64));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
				} break;
#endif
				case NC_CHAR:
				case NC_STRING: {
					attribute.value.s = new char[len + 1];
					attribute.value.s[len] = 0;
					NC_CALL(nc_get_att_text(parent_id, var_id, name, attribute.value.s));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				default:
					throw std::runtime_error("Variable type not supported yet");
				}

				result.attributes.push_back(attribute);
			}
            return result;
		},
		[return_type] (Napi::Env env, NCAttribute_list result) {
			
			Napi::Object attributes = Napi::Object::New(env);
			for (auto nc_attribute= result.attributes.begin(); nc_attribute != result.attributes.end(); nc_attribute++){
				Napi::Value value;
				if(nc_attribute->len == 1){
					switch (nc_attribute->type) {
					case NC_BYTE:{
						value = Napi::Number::New(env, nc_attribute->value.i8[0]);
					} break;
					case NC_SHORT:{
						value = Napi::Number::New(env, nc_attribute->value.i16[0]);
					} break;
					case NC_INT:{
						value = Napi::Number::New(env, nc_attribute->value.i32[0]);
					} break;
					case NC_FLOAT:{
						value = Napi::Number::New(env, nc_attribute->value.f[0]);
					} break;
					case NC_DOUBLE:{
						value = Napi::Number::New(env, nc_attribute->value.d[0]);
						
					} break;
					case NC_UBYTE:{
						value = Napi::Number::New(env, nc_attribute->value.u8[0]);
					} break;
					case NC_USHORT:{
						value = Napi::Number::New(env, nc_attribute->value.u16[0]);
					} break;
					case NC_UINT:{
						value = Napi::Number::New(env, nc_attribute->value.u32[0]);
					} break;
					case NC_UINT64:{
						value = Napi::BigInt::New(env, nc_attribute->value.u64[0]);
					} break;
					case NC_INT64:{
						value = Napi::BigInt::New(env, nc_attribute->value.i64[0]);						
					} break;
					case NC_CHAR:
					case NC_STRING:{
						 value = Napi::String::New(env, nc_attribute->value.s);
					} break;
					}	
				} else {
					switch (nc_attribute->type) {
					case NC_BYTE:{
						value = Napi::Int8Array::New(env, sizeof(int8_t),
							Napi::ArrayBuffer::New(env, nc_attribute->value.i8, nc_attribute->len * sizeof(int8_t)), 0, napi_int8_array);
					} break;
					case NC_SHORT:{
						value = Napi::Int16Array::New(env, sizeof(int16_t),
							Napi::ArrayBuffer::New(env, nc_attribute->value.i16, nc_attribute->len * sizeof(int16_t)), 0, napi_int8_array);
					} break;
					case NC_INT:{
						value = Napi::Int32Array::New(env, sizeof(int32_t),
							Napi::ArrayBuffer::New(env, nc_attribute->value.i32, nc_attribute->len * sizeof(int32_t)), 0, napi_int8_array);
					} break;
					case NC_FLOAT:{
						value = Napi::Float32Array::New(env, sizeof(float),
							Napi::ArrayBuffer::New(env, nc_attribute->value.f, nc_attribute->len * sizeof(float)), 0, napi_int8_array);
					} break;
					case NC_DOUBLE:{
						value = Napi::Float64Array::New(env, sizeof(double),
							Napi::ArrayBuffer::New(env, nc_attribute->value.d, nc_attribute->len * sizeof(double)), 0, napi_int8_array);						
					} break;
					case NC_UBYTE:{
						value = Napi::Uint8Array::New(env, sizeof(uint8_t),
							Napi::ArrayBuffer::New(env, nc_attribute->value.u8, nc_attribute->len * sizeof(uint8_t)), 0, napi_int8_array);	
					} break;
					case NC_USHORT:{
						value = Napi::Uint16Array::New(env, sizeof(uint16_t),
							Napi::ArrayBuffer::New(env, nc_attribute->value.u16, nc_attribute->len * sizeof(uint16_t)), 0, napi_int8_array);
					} break;
					case NC_UINT:{
						value = Napi::Uint32Array::New(env, sizeof(uint32_t),
							Napi::ArrayBuffer::New(env, nc_attribute->value.u32, nc_attribute->len * sizeof(uint32_t)), 0, napi_int8_array);
					} break;
					case NC_UINT64:{
						value = Napi::BigUint64Array::New(env, sizeof(uint64_t),
							Napi::ArrayBuffer::New(env, nc_attribute->value.u64, nc_attribute->len * sizeof(uint64_t)), 0, napi_int8_array);
					} break;
					case NC_INT64:{
						value = Napi::BigInt64Array::New(env, sizeof(int64_t),
							Napi::ArrayBuffer::New(env, nc_attribute->value.u64, nc_attribute->len * sizeof(int64_t)), 0, napi_int8_array);					
					} break;
					case NC_CHAR:
					case NC_STRING:{
						value = Napi::String::New(env, nc_attribute->value.s);
					} break;
					}
				}
				if(return_type){
					Napi::Object types_value = Napi::Object::New(env);
					types_value.Set(Napi::String::New(env, "type"),Napi::String::New(env, get_type_string(nc_attribute->type)) );
					types_value.Set(Napi::String::New(env, "value"), value);
					attributes.Set(Napi::String::New(env, nc_attribute->name), types_value);
				} else {
					attributes.Set(Napi::String::New(env, nc_attribute->name), value);
				}
			}
			return attributes;
		}
		
	);
	worker->Queue();

    return worker->Deferred();
}

}