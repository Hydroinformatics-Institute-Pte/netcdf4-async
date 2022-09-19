#include "Attribute.h"
#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"
#include "utils.h"
#include <node_version.h>
#include "Macros.h"

namespace netcdf4async {

struct NCAttribute_list
{
	std::vector<Item> attributes;
};

Napi::Value attr2value(Napi::Env env, Item *nc_item) {
	Napi::Value value;
//	printf("Attr %s type %i\n",nc_attribute->name.c_str(),nc_attribute->type);
	switch (nc_item->type) {
		case NC_BYTE:
			ITEM_TO_VAL(int8_t);
		break;
		case NC_SHORT:
			ITEM_TO_VAL(int16_t);
		break;
		case NC_INT:
			ITEM_TO_VAL(int32_t);
		break;
		case NC_FLOAT:
			ITEM_TO_VAL(float);
		break;
		case NC_DOUBLE:
			ITEM_TO_VAL(double);
		break;
		case NC_UBYTE:
			ITEM_TO_VAL(uint8_t);
		break;
		case NC_USHORT:
			ITEM_TO_VAL(uint16_t);
		break;
		case NC_UINT:
			ITEM_TO_VAL(uint32_t);
		break;
		case NC_UINT64:
			ITEM_TO_VAL(uint64_t);
		break;
		case NC_INT64:
			ITEM_TO_VAL(int64_t);
		break;
		case NC_CHAR: 
			value = Napi::String::New(env, nc_item->value.s);
			delete[] nc_item->value.s;
		break;
		case NC_STRING:
			if (nc_item->len == 1) {
				std::string *s = new std::string(nc_item->value.ps[0]);
				value = Napi::String::New(env, s->c_str());
				delete s;
			}
			else {
				Napi::Array result_array = Napi::Array::New(env, nc_item->len);
				for (int i = 0; i < static_cast<int>(nc_item->len); i++){
					std::string *res_str=new std::string(nc_item->value.ps[i]);
					result_array[i] = Napi::String::New(env, res_str->c_str());
					delete res_str;
				}
				value = result_array;
			}
		break;
	}	
	return value;
}


Napi::Value get_attributes(Napi::Env env, int parent_id, int var_id, bool return_type) {
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
				Item attribute;
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
				if (nc_attribute->type == NC_STRING) {
					NC_CALL(nc_free_string(nc_attribute->len,nc_attribute->value.ps));
					delete[] nc_attribute->value.ps;
				}
			}
			return attributes;
		}
	);
	worker->Queue();

    return worker->Deferred().Promise();
}

Napi::Value add_attribute(Napi::Env env, Napi::Promise::Deferred deferred, int parent_id, int var_id,
    const std::string attribute_name, int type, const Napi::Value &value) {
		Item nc_item;
		nc_item.name = attribute_name;
		nc_item.type = type;
		switch (type) {
			case NC_BYTE: 
				VAL_TO_ITEM(int8_t)
				break;
			case NC_SHORT: 
				VAL_TO_ITEM(int16_t)
				break;
			case NC_INT: 
				VAL_TO_ITEM(int32_t)
				break;
			case NC_FLOAT: 
				VAL_TO_ITEM(float)
				break;
			case NC_DOUBLE: 
				VAL_TO_ITEM(double)
				break;
			case NC_UBYTE: 
				VAL_TO_ITEM(uint8_t)
				break;
			case NC_USHORT: 
				VAL_TO_ITEM(uint16_t)
				break;
			case NC_UINT: 
				VAL_TO_ITEM(uint32_t)
				break;
#if NODE_MAJOR_VERSION > 8
			case NC_UINT64: 
				VAL_TO_ITEM(uint64_t)
				break;
			case NC_INT64: 
				VAL_TO_ITEM(int64_t)
				break;
#endif
			case NC_CHAR:{
				std::string v = value.As<Napi::String>().ToString();
				nc_item.value.s = new char[v.length()+1];
				nc_item.value.s[v.length()] = 0;
				strcpy(nc_item.value.s, v.c_str());
				nc_item.len = v.length();
			} break;
			case NC_STRING: {
				std::vector<std::unique_ptr<const std::string > >* string = new std::vector<std::unique_ptr<const std::string > >() ;
				std::vector<const char*>* cstrings = new std::vector<const char*>();
				if(value.IsArray()){
					auto arr = value.As<Napi::Array>();
					nc_item.len = static_cast<int>(arr.Length());
					for (int i =0; i<static_cast<int>(arr.Length()); i++){
						Napi::Value napiV=arr[i];
						string->push_back(std::make_unique<std::string>(std::string(napiV.ToString().Utf8Value())));
						cstrings->push_back(string->at(i)->c_str());
					}
				} else {
					string->push_back(std::make_unique<std::string>(std::string(value.As<Napi::String>().ToString().Utf8Value())));
					cstrings->push_back(string->at(0)->c_str());
					nc_item.len = 1;
				}
				nc_item.value.v = cstrings->data();
				
			} break;
			default:{
				deferred.Reject(Napi::String::New(env, "Variable type not supported yet"));
				return deferred.Promise();
			}
		}

		auto worker = new NCAsyncWorker<Item>(
		env, deferred,
		[parent_id, var_id, nc_item] (const NCAsyncWorker<Item>* worker) {
			
			if(nc_item.type == NC_CHAR ) {
				std::string text = std::string(nc_item.value.s);
				NC_CALL(nc_put_att_text(parent_id, var_id, nc_item.name.c_str(),
					text.length(), text.c_str()));
			} if(nc_item.type == NC_STRING ) {
				NC_CALL(nc_put_att(parent_id, var_id,nc_item.name.c_str(), nc_item.type, nc_item.len, nc_item.value.v));
			} else {
				NC_CALL(nc_put_att(parent_id, var_id,nc_item.name.c_str(), nc_item.type, nc_item.len, nc_item.value.v));
				
			}
			return nc_item;
		},
		[] (Napi::Env env, Item result) {
			Napi::Value value=attr2value(env,&result);
			
			Napi::Object types_value = Napi::Object::New(env);
			types_value.Set(Napi::String::New(env, "type"),Napi::String::New(env, get_type_string(result.type)) );
			types_value.Set(Napi::String::New(env, "value"), value);
			Napi::Object attribute = Napi::Object::New(env);
			attribute.Set(Napi::String::New(env, result.name), types_value);
			return attribute;
		}
	);
	worker->Queue();
	return worker->Deferred().Promise();
}


Napi::Value delete_attribute(Napi::Env env, int parent_id, int var_id, const std::string name) {
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
	return worker->Deferred().Promise();
}


Napi::Value rename_attribute(Napi::Env env, Napi::Promise::Deferred deferred, int parent_id, int var_id, const std::string old_name, const std::string new_name) {

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
	return worker->Deferred().Promise();
}

}