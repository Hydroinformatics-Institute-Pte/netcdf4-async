#include "Attribute.h"
#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"
#include "utils.h"

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
};
struct attr_struct
{
	std::string name;
	int type;
	size_t len;
	UnionType value;
};



struct NCAttribute_list
{
	std::vector<attr_struct> attributes;
};

Napi::Value attr2value(Napi::Env env,attr_struct *nc_attribute) {
	Napi::Value value;
	// printf("Attr %s type %i\n",nc_attribute->name.c_str(),nc_attribute->type);
	switch (nc_attribute->type) {
		case NC_BYTE:
			// printf("Value byte %i\n",nc_attribute->value.i8[0]);
			ATTR_TO_VAL(i8,int8_t,Number,Int8Array);
		break;
		case NC_SHORT:
			// printf("Value short %i\n",nc_attribute->value.i16[0]);
			ATTR_TO_VAL(i16,int16_t,Number,Int16Array);
		break;
		case NC_INT:
			// printf("Value int %i\n",nc_attribute->value.i32[0]);
			ATTR_TO_VAL(i32,int32_t,Number,Int32Array);
		break;
		case NC_FLOAT:
			// printf("Value float %f\n",nc_attribute->value.f[0]);
			ATTR_TO_VAL(f,float,Number,Float32Array);
		break;
		case NC_DOUBLE:
			// printf("Value double %lf\n",nc_attribute->value.d[0]);
			ATTR_TO_VAL(d,double,Number,Float64Array);
		break;
		case NC_UBYTE:
			ATTR_TO_VAL(u8,uint8_t,Number,Uint8Array);
		break;
		case NC_USHORT:
			ATTR_TO_VAL(u16,uint16_t,Number,Uint16Array);
		break;
		case NC_UINT:
			ATTR_TO_VAL(u32,uint32_t,Number,Uint32Array);
		break;
		case NC_UINT64:
			ATTR_TO_VAL(u64,uint64_t,BigInt,BigUint64Array);
		break;
		case NC_INT64:
			ATTR_TO_VAL(i64,int64_t,BigInt,BigInt64Array);
		break;
		case NC_CHAR: 
			value = Napi::String::New(env, nc_attribute->value.s);
			delete[] nc_attribute->value.s;
		break;
		case NC_STRING:
			if (nc_attribute->len == 1) {
				printf("string = %s\n", nc_attribute->value.s);
				std::string s = std::string(nc_attribute->value.s);
				printf("string = %s\n", nc_attribute->value.s);
				value = Napi::String::New(env, s.c_str());
				//NC_VOID_CALL(nc_free_string(nc_attribute->len,&nc_attribute->value.s));
				//delete[] nc_attribute->value.s;
			}
			else {
				Napi::Array result_array = Napi::Array::New(env, nc_attribute->len);
				for (int i = 0; i < static_cast<int>(nc_attribute->len); i++){
					std::string *res_str=new std::string(nc_attribute->value.ps[i]);
					result_array[i] = Napi::String::New(env, res_str->c_str());
					delete res_str;
				}
				value = result_array;
				NC_VOID_CALL(nc_free_string(nc_attribute->len,nc_attribute->value.ps));
				delete[] nc_attribute->value.ps;
			}
		break;
	}	
	return value;
}


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
				case NC_BYTE: 
					attribute.value.i8 = new int8_t[len];
				break;
				case NC_SHORT: 
					attribute.value.i16 = new int16_t[len];
				break;
				case NC_INT: 
					attribute.value.i32 = new int32_t[len];
				break;
				case NC_FLOAT: 
					attribute.value.f = new float[len];
				break;
				case NC_DOUBLE:
					attribute.value.d = new double[len];
				break;
				case NC_UBYTE: 
					attribute.value.u8 = new uint8_t[len];
				break;
				case NC_USHORT: 
					attribute.value.u16 = new uint16_t[len];
				break;
				case NC_UINT: 
					attribute.value.u32 = new uint32_t[len];
				break;
#if NODE_MAJOR_VERSION > 8
				case NC_UINT64: 
					attribute.value.u64 = new uint64_t[len];
				break;
				case NC_INT64:
					attribute.value.i64 = new int64_t[len];
				break;
#endif
				case NC_CHAR: 
					attribute.value.s = new char[len + 1];
					attribute.value.s[len] = 0;
				break;
				case NC_STRING: 
					attribute.value.ps = new char*[len];
				break;
				default:
					throw std::runtime_error("Variable type not supported yet");
				}
				NC_CALL(nc_get_att(parent_id, var_id, name, attribute.value.v));
				
				result.attributes.push_back(attribute);
			}
            return result;
		},
		[return_type] (Napi::Env env, NCAttribute_list result) {
			
			Napi::Object attributes = Napi::Object::New(env);
			for (auto nc_attribute= result.attributes.begin(); nc_attribute != result.attributes.end(); nc_attribute++){
				Napi::Value value=attr2value(env,&*nc_attribute);
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
				VAL_TO_ATTR(int8_t, Number, Int32Value);
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(int8_t, BigInt, Int64Value);
#endif
			} else {
				auto array = value.As<Napi::Int8Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_SHORT: {
			if (value.IsNumber()) {
				VAL_TO_ATTR(int16_t, Number, Int32Value);
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(int16_t, BigInt, Int64Value);
#endif
			} else {
				auto array = value.As<Napi::Int16Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_INT: {
			if (value.IsNumber()) {
				VAL_TO_ATTR(int32_t, Number, Int32Value);
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(int32_t, BigInt, Int64Value);
#endif
			} else {
				auto array = value.As<Napi::Int32Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_FLOAT: {
			if (value.IsNumber()) {
				VAL_TO_ATTR(float, Number, FloatValue);
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(float, BigInt, Int64Value);
#endif
			} else {
				auto array = value.As<Napi::Float32Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_DOUBLE: {
			if (value.IsNumber()) {
				VAL_TO_ATTR(double, Number, DoubleValue);
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(double, BigInt, Int64Value);
#endif
			} else {
				auto array = value.As<Napi::Float64Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_UBYTE: {
			if (value.IsNumber()) {
				VAL_TO_ATTR(uint8_t, Number, Uint32Value);
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(uint8_t, BigInt, Uint64Value);
#endif
			} else {
				auto array = value.As<Napi::Uint8Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_USHORT: {
			if (value.IsNumber()) {
				VAL_TO_ATTR(uint16_t, Number, Uint32Value);
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(uint16_t, BigInt, Uint64Value);
#endif
			} else {
				auto array = value.As<Napi::Uint16Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_UINT: {
			if (value.IsNumber()) {
				VAL_TO_ATTR(uint32_t, Number, Uint32Value);
#if NODE_MAJOR_VERSION > 8
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(uint32_t, BigInt, Uint64Value);
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
				VAL_TO_ATTR(uint64_t, Number, Uint64Value);
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(uint64_t, BigInt, Uint64Value);
			} else {
				auto array = value.As<Napi::BigUint64Array>();
				attribute_value.len = array.ElementLength();
				attribute_value.value.v = array.ArrayBuffer().Data();
			}
		} break;
		case NC_INT64: {
			if (value.IsNumber()) {
				VAL_TO_ATTR(uint64_t, Number, Int64Value);
			} else if (value.IsBigInt()) {
				VAL_TO_BIG_ATTR(int64_t, BigInt, Int64Value);
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
			printf("_______________ STRING_________________________\n");
			std::vector<std::unique_ptr<const std::string > > string{};
			std::vector<const char*> cstrings{};
			if(value.IsArray()){
				auto arr = value.As<Napi::Array>();
				printf("array len = %d",static_cast<int>(arr.Length()));
				attribute_value.len = static_cast<int>(arr.Length());
				printf("array len = %d",static_cast<int>(attribute_value.len));
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
			printf("after upt att\n");
			Napi::Value value=attr2value(env,&result);
			
			Napi::Object types_value = Napi::Object::New(env);
			types_value.Set(Napi::String::New(env, "type"),Napi::String::New(env, get_type_string(result.type)) );
			types_value.Set(Napi::String::New(env, "value"), value);
			Napi::Object attribute = Napi::Object::New(env);
			attribute.Set(Napi::String::New(env, result.name), types_value);
			printf("return atrr\n");
			return attribute;
		}
	);
	worker->Queue();
	return worker->Deferred();
	}


Napi::Promise::Deferred delete_attribute(Napi::Env env, int parent_id, int var_id, std::string name) {
	auto worker = new NCAsyncWorker<std::string>(
		env,
		[parent_id, var_id, name] (const NCAsyncWorker<std::string>* worker) {
			
			NC_CALL(nc_del_att(parent_id, var_id, name.c_str()));
			
            return name;
		},
		[] (Napi::Env env, std::string result) {
			return Napi::String::New(env, result);
		}
		
	);
	worker->Queue();
	return worker->Deferred();
}


Napi::Promise::Deferred rename_attribute(Napi::Env env, int parent_id, int var_id, std::string old_name, std::string new_name) {

	auto worker = new NCAsyncWorker<std::string>(
		env,
		[parent_id, var_id, old_name, new_name] (const NCAsyncWorker<std::string>* worker) {
			
			NC_CALL(nc_rename_att(parent_id, var_id, old_name.c_str(), new_name.c_str()));
			
            return new_name;
		},
		[] (Napi::Env env, std::string result) {
			return Napi::String::New(env, result);
		}
		
	);
	worker->Queue();
	return worker->Deferred();
}

}