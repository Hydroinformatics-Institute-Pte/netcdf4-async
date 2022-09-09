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
		char** ps;
		const char* text;
		void* v;
	};
	UnionType value;
};


struct NCAttribute_list
{
	std::vector<attr_struct> attributes;
};

Napi::Promise::Deferred get_attributes(Napi::Env env, int parent_id, int var_id, bool return_type) {
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
				attribute.len = len;
				attribute.name = std::string(name);
				attribute.type = type;
				
				switch (type) {
				case NC_BYTE: {
					attribute.value.i8 = new int8_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.i8));
				} break;
				case NC_SHORT: {
					attribute.value.i16 = new int16_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.i16));
				} break;
				case NC_INT: {
					attribute.value.i32 = new int32_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.i32));
				} break;
				case NC_FLOAT: {
					attribute.value.f = new float[len];
					NC_CALL(nc_get_att(parent_id, var_id, name,attribute.value.f));
				} break;
				case NC_DOUBLE: {
					attribute.value.d = new double[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.d));
				} break;
				case NC_UBYTE: {
					attribute.value.u8 = new uint8_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.u8));
				} break;
				case NC_USHORT: {
					attribute.value.u16 = new uint16_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.u16));
				} break;
				case NC_UINT: {
					attribute.value.u32 = new uint32_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.u32));
				} break;
#if NODE_MAJOR_VERSION > 8
				case NC_UINT64: {
					attribute.value.u64 = new uint64_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name,attribute.value.u64));
				} break;
				case NC_INT64: {
					attribute.value.i64 = new int64_t[len];
					NC_CALL(nc_get_att(parent_id, var_id, name,attribute.value.i64));
				} break;
#endif
				case NC_CHAR: {
					attribute.value.s = new char[len + 1];
					attribute.value.s[len] = 0;
					NC_CALL(nc_get_att_text(parent_id, var_id, name, attribute.value.s));
				} break;
				case NC_STRING: {
					attribute.value.ps = new char*[len];
        			NC_CALL(nc_get_att_string(parent_id, var_id, name, attribute.value.ps));
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
					case NC_CHAR: {
						value = Napi::String::New(env, nc_attribute->value.s);
					} break;
					case NC_STRING:{
						value = Napi::String::New(env, nc_attribute->value.ps[0]);
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
					case NC_CHAR: {
						value = Napi::String::New(env, nc_attribute->value.s);
					} break;
					case NC_STRING:{
						Napi::Array result_array = Napi::Array::New(env, nc_attribute->len);
        				for (int i = 0; i < static_cast<int>(nc_attribute->len); i++){
							std::string *res_str=new std::string(nc_attribute->value.ps[i]);
           					result_array[i] = Napi::String::New(env, res_str->c_str());
							delete res_str;
        				}
						value = result_array;
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

Napi::Promise::Deferred add_attribute(Napi::Promise::Deferred deferred , Napi::Env env, int parent_id, int var_id,
    const std::string attribute_name, int type, const Napi::Value &value) {
		attr_struct attribute_value;
		attribute_value.name = attribute_name;
		attribute_value.type = type;
		switch (type) {
		case NC_BYTE: {
			if (value.IsNumber()) {
				int8_t v = value.As<Napi::Number>().Int32Value();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				int8_t v = value.As<Napi::BigInt>().Int64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#endif
			} else {
				auto array = value.As<Napi::Int8Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_SHORT: {
			if (value.IsNumber()) {
				int16_t v = value.As<Napi::Number>().Int32Value();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				int16_t v = value.As<Napi::BigInt>().Int64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#endif
			} else {
				auto array = value.As<Napi::Int16Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_INT: {
			if (value.IsNumber()) {
				int32_t v = value.As<Napi::Number>().Int32Value();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				int32_t v = value.As<Napi::BigInt>().Int64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#endif
			} else {
				auto array = value.As<Napi::Int32Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_FLOAT: {
			if (value.IsNumber()) {
				float v = value.As<Napi::Number>().FloatValue();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				float v = value.As<Napi::BigInt>().Int64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v= &v;
#endif
			} else {
				auto array = value.As<Napi::Float32Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_DOUBLE: {
			if (value.IsNumber()) {
				double v = value.As<Napi::Number>().DoubleValue();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				double v = value.As<Napi::BigInt>().Int64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#endif
			} else {
				auto array = value.As<Napi::Float64Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_UBYTE: {
			if (value.IsNumber()) {
				uint8_t v = value.As<Napi::Number>().Uint32Value();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				uint8_t v = value.As<Napi::BigInt>().Uint64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#endif
			} else {
				auto array = value.As<Napi::Uint8Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_USHORT: {
			if (value.IsNumber()) {
				uint16_t v = value.As<Napi::Number>().Uint32Value();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				uint16_t v = value.As<Napi::BigInt>().Uint64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#endif
			} else {
				auto array = value.As<Napi::Uint16Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_UINT: {
			if (value.IsNumber()) {
				uint32_t v = value.As<Napi::Number>().Uint32Value();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				uint32_t v = value.As<Napi::BigInt>().Uint64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
#endif
			} else {
				auto array = value.As<Napi::Uint32Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
#if NODE_MAJOR_VERSION > 8

		case NC_UINT64: {
			if (value.IsNumber()) {
				uint64_t v = value.As<Napi::Number>().Uint32Value();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
			} else if (value.IsBigInt()) {
				uint64_t v = value.As<Napi::BigInt>().Uint64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
			} else {
				auto array = value.As<Napi::BigUint64Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_INT64: {
			if (value.IsNumber()) {
				int64_t v = value.As<Napi::Number>().Uint32Value();
				attribute_value.len = 1;
				attribute_value.value.v = &v;
			} else if (value.IsBigInt()) {
				int64_t v = value.As<Napi::BigInt>().Uint64Value(nullptr);
				attribute_value.len = 1;
				attribute_value.value.v = &v;
			} else {
				auto array = value.As<Napi::BigInt64Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
#endif
		case NC_CHAR:{
			std::string v = value.As<Napi::String>().ToString();
			attribute_value.value.s = new char[v.length()+1];
			attribute_value.value.s[v.length()] = 0;
			strcpy(attribute_value.value.s, v.c_str());
			attribute_value.len = v.length();
		} break;
		case NC_STRING: {
			std::vector<std::unique_ptr<const std::string > > string{};
			std::vector<const char*> cstrings{};
			if(value.IsArray()){
				auto arr = value.As<Napi::Array>();
				attribute_value.len = static_cast<int>(arr.Length());
				for (int i =0; i<static_cast<int>(arr.Length()); i++){
					Napi::Value napiV=arr[i];
					string.push_back(std::make_unique<std::string>(std::string(napiV.ToString())));
					cstrings.push_back(string.at(i)->c_str());
				}
				attribute_value.value.v = cstrings.data();
			} else {
				std::string v = value.As<Napi::String>().ToString();
				attribute_value.value.s = new char[v.length()+1];
				attribute_value.value.s[v.length()] = 0;
				strcpy(attribute_value.value.s, v.c_str());
				attribute_value.len = 1;
			}
			
		} break;
		default:{
			deferred.Reject(Napi::String::New(env, "Variable type not supported yet"));
			return deferred;
		}
		}

		auto worker = new NCAsyncWorker<attr_struct>(
		env, deferred,
		[parent_id, var_id, attribute_value] (const NCAsyncWorker<attr_struct>* worker) {
			attr_struct result;
			result.name = attribute_value.name;
			result.len = attribute_value.len;
			result.type = attribute_value.type;
			result.value = attribute_value.value;
			if(attribute_value.type == NC_CHAR ) {
				std::string text = std::string(attribute_value.value.s);
				NC_CALL(nc_put_att_text(parent_id, var_id, attribute_value.name.c_str(),
					text.length(), text.c_str()));
			} if(attribute_value.type == NC_STRING ) {
				if (attribute_value.len == 1) {
					NC_CALL(nc_put_att_string(parent_id, var_id,attribute_value.name.c_str(), attribute_value.len, const_cast<const char**>(&attribute_value.value.s)));	
				} else {
					NC_CALL(nc_put_att_string(parent_id, var_id,attribute_value.name.c_str(), attribute_value.len, static_cast<const char **>(attribute_value.value.v)));
				}	
			} else {
				NC_CALL(nc_put_att(parent_id, var_id,attribute_value.name.c_str(), attribute_value.type, attribute_value.len, attribute_value.value.v));
			}
			return result;
		},
		[] (Napi::Env env, attr_struct result) {
			Napi::Value value;
			if(result.len == 1){
				switch (result.type) {
				case NC_BYTE:
				case NC_SHORT:
				case NC_INT:
				case NC_FLOAT:
				case NC_DOUBLE:
				case NC_UBYTE:
				case NC_USHORT:
				case NC_UINT:{
					value = Napi::Number::New(env, *static_cast<double*>(result.value.v));
				} break;
				case NC_UINT64:
				case NC_INT64:{
					value = Napi::BigInt::New(env, *static_cast<int64_t*>(result.value.v));						
				} break;
				case NC_CHAR:
				case NC_STRING:{
					 value = Napi::String::New(env, result.value.s);
				} break;
				}	
			} else {
				switch (result.type) {
				case NC_BYTE:{
					value = Napi::Int8Array::New(env, sizeof(int8_t),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(int8_t)), 0, napi_int8_array);
				} break;
				case NC_SHORT:{
					value = Napi::Int16Array::New(env, sizeof(int16_t),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(int16_t)), 0, napi_int8_array);
				} break;
				case NC_INT:{
					value = Napi::Int32Array::New(env, sizeof(int32_t),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(int32_t)), 0, napi_int8_array);
				} break;
				case NC_FLOAT:{
					value = Napi::Float32Array::New(env, sizeof(float),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(float)), 0, napi_int8_array);
				} break;
				case NC_DOUBLE:{
					value = Napi::Float64Array::New(env, sizeof(double),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(double)), 0, napi_int8_array);						
				} break;
				case NC_UBYTE:{
					value = Napi::Uint8Array::New(env, sizeof(uint8_t),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(uint8_t)), 0, napi_int8_array);	
				} break;
				case NC_USHORT:{
					value = Napi::Uint16Array::New(env, sizeof(uint16_t),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(uint16_t)), 0, napi_int8_array);
				} break;
				case NC_UINT:{
					value = Napi::Uint32Array::New(env, sizeof(uint32_t),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(uint32_t)), 0, napi_int8_array);
				} break;
				case NC_UINT64:{
					value = Napi::BigUint64Array::New(env, sizeof(uint64_t),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(uint64_t)), 0, napi_int8_array);
				} break;
				case NC_INT64:{
					value = Napi::BigInt64Array::New(env, sizeof(int64_t),
						Napi::ArrayBuffer::New(env, result.value.v, result.len * sizeof(int64_t)), 0, napi_int8_array);					
				} break;
				case NC_CHAR:
				case NC_STRING:{
					value = Napi::String::New(env, result.value.s);
				} break;
				}

			}
			Napi::Object types_value = Napi::Object::New(env);
			types_value.Set(Napi::String::New(env, "type"),Napi::String::New(env, get_type_string(result.type)) );
			types_value.Set(Napi::String::New(env, "value"), value);
			Napi::Object attribute = Napi::Object::New(env);
			attribute.Set(Napi::String::New(env, result.name), types_value);
			return attribute;
		}
	);
	worker->Queue();
	return worker->Deferred();
	}
}