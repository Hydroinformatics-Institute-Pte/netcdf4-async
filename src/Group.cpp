#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"

namespace netcdf4async {

/**
 * @brief Group operation results
 * 
 */
struct NCGroup_result
{
	/// @brief File id
	int id;
	/// @brief Group name
	std::string name;
}; 

template <class NCResult> struct NCGroup_list
{
	std::vector<NCResult> groups;
};

struct NCGroup_dims
{
	/// @brief File id
	int id;
	/// @brief Group name
	std::string name;
	/// @brief Dimension len or NC_UNLIMITED for unlimited
	size_t len;
};

struct Attribute
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
	std::vector<Attribute> attributes;
};

Napi::FunctionReference Group::constructor;

Napi::Object Group::Build(Napi::Env env, int id,std::string name) {
	return constructor.New({
		Napi::Number::New(env, id),
		Napi::String::New(env,name)
	});
}

Group::Group(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Group>(info) {
	if (info.Length() < 2) {
		Napi::TypeError::New(info.Env(), "Wrong number of arguments").ThrowAsJavaScriptException();
		return;
	}

	id = info[0].As<Napi::Number>().Int32Value();
	name = info[1].As<Napi::String>().Utf8Value();
}

void Group::Init(Napi::Env env) {

	Napi::HandleScope scope(env);

    Napi::Function func =
		DefineClass(env, "Group",
			{
                InstanceMethod("getName", &Group::GetName),
                InstanceMethod("setName", &Group::SetName),
                InstanceMethod("getPath", &Group::GetPath),

		        InstanceMethod("getSubgroups",&Group::GetSubgroups),
		        InstanceMethod("getSubgroup",&Group::GetSubgroup),
		        InstanceMethod("addSubgroup", &Group::AddSubgroup),


		        InstanceMethod("getDimensions",&Group::GetDimensions),
		        InstanceMethod("addDimension", &Group::AddDimension),
//				InstanceMethod("renameDimension",&Group::RenameDimension),

		        InstanceMethod("getAttributes", &Group::GetAttributes),
		        InstanceMethod("addAttribute", &Group::AddAttribute),

		        InstanceMethod("getVariables",&Group::GetVariables),
				InstanceMethod("addVariable", &Group::AddVariable),
		        InstanceMethod("inspect", &Group::Inspect)
//                InstanceAccessor<&Group::GetId>("id"),
			}
		);
    constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();
}

Napi::Value Group::AddAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	// if (info.Length() != static_cast<size_t>(3)) {
	// 	Napi::TypeError::New(info.Env(), "Not all parameters from name,type,value bound").ThrowAsJavaScriptException();
	// 	return info.Env().Undefined();
	// }
	// std::string type_str=info[1].As<Napi::String>().ToString();
	// int type=get_type(type_str);
	// std::string name=info[0].As<Napi::String>().ToString();
	// Attribute::set_value(info,this->id,NC_GLOBAL,name,type,info[2]);
	// return Attribute::Build(info.Env(),name,NC_GLOBAL,this->id,type);
    return deferred.Promise();
}

Napi::Value Group::AddSubgroup(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (info.Length() != static_cast<size_t>(1)) {
		deferred.Reject(Napi::String::New(info.Env(),"Missing subgroup name"));
		return deferred.Promise();
	}
	std::string new_name = info[0].As<Napi::String>().ToString();
	Napi::Env env = info.Env();
	int id = this->id;

	(new NCAsyncWorker<NCGroup_result>(
		env,
		deferred,
		[id, new_name] (const NCAsyncWorker<NCGroup_result>* worker) {
			int new_id;
			NC_CALL(nc_def_grp(id, new_name.c_str(),&new_id));
			static NCGroup_result result;
            result.id = new_id;
			result.name = new_name;
            return result;
		},
		[] (Napi::Env env, NCGroup_result result) {
			Napi::Object group = Group::Build(env, result.id,result.name);
         	return group;
		}
		
	))->Queue();
    return deferred.Promise();
}

Napi::Value Group::AddDimension(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	Napi::Env env = info.Env();
	if (info.Length() != static_cast<size_t>(2)) {
	 	deferred.Reject(Napi::String::New(env, "Wrong number of arguments. Need dimension name and length"));
	    return deferred.Promise();
	}
	int len;
	const std::string len_symbol=info[1].As<Napi::String>().ToString();
	if (len_symbol=="unlimited") {
	 	len=NC_UNLIMITED;
	}
	else {
	 	len=info[1].As<Napi::Number>().Int32Value();
	 	if (len<=0) {
			deferred.Reject(Napi::String::New(env, "Expected positive integer as dimension length"));
			return deferred.Promise();
	 	}
	}
	const std::string name = info[0].As<Napi::String>().ToString();
	auto worker=new NCAsyncWorker<NCGroup_dims>(
		env,
		deferred,
		[id=this->id,len,name] (const NCAsyncWorker<NCGroup_dims>* worker) {
			NCGroup_dims result;
			NC_CALL(nc_def_dim(id,name.c_str(),len,&result.id));
			result.name=name;
			result.len=len;
			return result;
		},
		[] (Napi::Env env,NCGroup_dims result) {
			Napi::Object dimension = Napi::Object::New(env);
			auto len = result.len==NC_UNLIMITED?Napi::String::New(env,"unlimited"):Napi::Number::New(env,result.len);
			dimension.Set(Napi::String::New(env,result.name), len);
         	return dimension;
		}

	);
	worker->Queue();

	return worker->Deferred().Promise(); 

}

Napi::Value Group::AddVariable(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	// if (info.Length() != static_cast<size_t>(3)) {
	// 	Napi::TypeError::New(info.Env(), "Wrong number of arguments. Need variable name, type and dimenisons").ThrowAsJavaScriptException();
	// 	return info.Env().Undefined();
	// }
	// if (!info[2].IsArray()) {
	// 	Napi::TypeError::New(info.Env(),"Dimensions must be an array").ThrowAsJavaScriptException();
	// 	return info.Env().Undefined();
	// }
	// auto dims=info[2].As<Napi::Array>();
	// auto dims_size=dims.Length();
	// if (dims_size==0u) {
	// 	Napi::TypeError::New(info.Env(),"Dimensions must be a non-empty array").ThrowAsJavaScriptException();
	// 	return info.Env().Undefined();
	// }
	// std::string type_str=info[1].As<Napi::String>().ToString();
	// int type=get_type(type_str);
	// if (type==NC2_ERR) {
	// 	Napi::TypeError::New(info.Env(),"Bad variable type").ThrowAsJavaScriptException();
	// 	return info.Env().Undefined();
	// }

	// std::string name=info[0].As<Napi::String>().ToString();
	// int *ndims = new int[dims_size];
	// for(auto i=0u;i<dims_size;i++) {
	// 	Napi::Value v=dims[i];
	// 	if (v.IsNumber()) {
	// 		ndims[i]=v.As<Napi::Number>().Int32Value();
	// 	}
	// 	else {
	// 		std::string dim_name=v.ToString();
	// 		NC_CALL(nc_inq_dimid(this->id,dim_name.c_str(),&ndims[i]));
	// 	}
	// }
	// int new_id;
	// NC_CALL(nc_def_var(this->id,name.c_str(),type,dims_size,ndims,&new_id));
	// delete[] ndims;
	// return Variable::Build(info.Env(),new_id,this->id);
    return deferred.Promise();
}

Napi::Value Group::GetId(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), id);
}

Napi::Value Group::GetVariables(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	// int nvars;
	// NC_CALL(nc_inq_varids(this->id, &nvars, NULL));
	// int *var_ids = new int[nvars];
	// NC_CALL(nc_inq_varids(this->id, NULL, var_ids));

	// Napi::Object vars = Napi::Object::New(info.Env());

	// char name[NC_MAX_NAME + 1];
	// for (int i = 0; i < nvars; ++i) {
	// 	Napi::Object var = Variable::Build(info.Env(), var_ids[i], this->id);

	// 	int retval = nc_inq_varname(this->id, var_ids[i], name);
	// 	if (retval == NC_NOERR) {
	// 		vars.Set(name, var);
	// 	}
	// }
	// delete[] var_ids;
	// return vars;
    return deferred.Promise();
}

Napi::Value Group::GetDimensions(const Napi::CallbackInfo &info) {
	bool unlimited=false;
	Napi::Env env = info.Env();
	if (info.Length() >= static_cast<size_t>(1)) {
		unlimited=info[0].As<Napi::Boolean>().Value();
	}
	auto worker=new NCAsyncWorker<NCGroup_list<NCGroup_dims>>(
		env,
		[id=this->id,unlimited] (const NCAsyncWorker<NCGroup_list<NCGroup_dims>>* worker) {
			int ndims;
			if (unlimited) {
				NC_CALL(nc_inq_unlimdims(id, &ndims, NULL));
			}
			else {
				NC_CALL(nc_inq_dimids(id, &ndims, NULL, 0));
			}
			int *dim_ids = new int[ndims];
			if (unlimited) {
				NC_CALL(nc_inq_unlimdims(id, NULL, dim_ids));
			}
			else {
				NC_CALL(nc_inq_dimids(id, NULL, dim_ids, 0));
			}
			NCGroup_list<NCGroup_dims> result;
			char name[NC_MAX_NAME + 1];
			size_t len;
			for (int i = 0; i < ndims; ++i) {
				int retval = nc_inq_dim(id, dim_ids[i], name,&len);
				if (retval == NC_NOERR) {
					NCGroup_dims dim;
					dim.id = dim_ids[i];
					dim.name = name;
					dim.len = len;
					result.groups.push_back(dim);
				}
			}
			return result;
		},
		[] (Napi::Env env,NCGroup_list<NCGroup_dims> result) {
			Napi::Object dimensions = Napi::Object::New(env);
			for (auto nc_dim= result.groups.begin(); nc_dim != result.groups.end(); nc_dim++){
				auto len = nc_dim->len==NC_UNLIMITED?Napi::String::New(env,"unlimited"):Napi::Number::New(env,nc_dim->len);
				dimensions.Set(Napi::String::New(env,nc_dim->name), len);
			}
         	return dimensions;
		}

	);
	worker->Queue();

	return worker->Deferred().Promise(); 

}


Napi::Value Group::GetAttributes(const Napi::CallbackInfo &info) {
	bool return_type = true;
	if(info.Length() >0) {
		return_type = info[0].As<Napi::Boolean>();
	}
	Napi::Env env = info.Env();
	int id = this->id;
    auto worker = new NCAsyncWorker<NCAttribute_list>(
		env,
		[id] (const NCAsyncWorker<NCAttribute_list>* worker) {
			int natts;
			NC_CALL(nc_inq_natts(id, &natts));
			char name[NC_MAX_NAME + 1];
			NCAttribute_list result;
			
			for (int i = 0; i < natts; ++i) {
				int type;
				size_t len;
				Attribute attribute;
				NC_CALL(nc_inq_attname(id, NC_GLOBAL, i, name));
				NC_CALL(nc_inq_attlen(id, NC_GLOBAL, name, &len));
				NC_CALL(nc_inq_atttype(id, NC_GLOBAL, name, &type));
				
				switch (type) {
				case NC_BYTE: {
					attribute.value.i8 = new int8_t[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name, attribute.value.i8));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				case NC_SHORT: {
					attribute.value.i16 = new int16_t[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name, attribute.value.i16));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
				
				} break;
				case NC_INT: {
					attribute.value.i32 = new int32_t[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name, attribute.value.i32));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				case NC_FLOAT: {
					attribute.value.f = new float[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name,attribute.value.f));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				case NC_DOUBLE: {
					attribute.value.d = new double[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name, attribute.value.d));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
				case NC_UBYTE: {
					attribute.value.u8 = new uint8_t[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name, attribute.value.u8));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
				} break;
				case NC_USHORT: {
					attribute.value.u16 = new uint16_t[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name, attribute.value.u16));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
				} break;
				case NC_UINT: {
					attribute.value.u32 = new uint32_t[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name, attribute.value.u32));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
					
				} break;
#if NODE_MAJOR_VERSION > 8
				case NC_UINT64: {
					attribute.value.u64 = new uint64_t[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name,attribute.value.u64));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
		
				} break;
				case NC_INT64: {
					attribute.value.i64 = new int64_t[len];
					NC_CALL(nc_get_att(id, NC_GLOBAL, name,attribute.value.i64));
					attribute.len = len;
					attribute.name = std::string(name);
					attribute.type = type;
				} break;
#endif
				case NC_CHAR:
				case NC_STRING: {
					attribute.value.s = new char[len + 1];
					attribute.value.s[len] = 0;
					NC_CALL(nc_get_att_text(id, NC_GLOBAL, name, attribute.value.s));
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

    return worker->Deferred().Promise();
}

Napi::Value Group::GetSubgroups(const Napi::CallbackInfo &info) {
//	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	Napi::Env env = info.Env();
	int id = this->id;
	auto worker=new NCAsyncWorker<NCGroup_list<NCGroup_result>>(
		env,
		[id] (const NCAsyncWorker<NCGroup_list<NCGroup_result>>* worker) {
			int ngrps;
			NC_CALL(nc_inq_grps(id, &ngrps, NULL));
			int *grp_ids = new int[ngrps];
			NC_CALL(nc_inq_grps(id, NULL, grp_ids));
			NCGroup_list<NCGroup_result> result;
			char name[NC_MAX_NAME + 1];
			for (int i = 0; i < ngrps; ++i) {
				int retval = nc_inq_grpname(grp_ids[i], name);
				if (retval == NC_NOERR) {
					NCGroup_result sub_group;
					sub_group.id = grp_ids[i];
					sub_group.name = name;
					result.groups.push_back(sub_group);
				}
			}

            return result;
		},
		[] (Napi::Env env,NCGroup_list<NCGroup_result> result) {
			Napi::Object subgroups = Napi::Object::New(env);
			for (auto nc_group= result.groups.begin(); nc_group != result.groups.end(); nc_group++){
				Napi::Object group = Group::Build(env, nc_group->id,nc_group->name);
				subgroups.Set(Napi::String::New(env,nc_group->name), group);
			}
         	return subgroups;
		}
		
	);
	worker->Queue();

	return worker->Deferred().Promise(); 
}


Napi::Value Group::GetSubgroup(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	Napi::Env env = info.Env();
	if (info.Length() != static_cast<size_t>(1)) {
			deferred.Reject(Napi::String::New(env, "Expected subgroup name"));
			return deferred.Promise();
	}

	std::string group_name=info[0].As<Napi::String>().ToString();
	auto worker=new NCAsyncWorker<NCGroup_result>(
		env,
		[id=this->id,group_name] (const NCAsyncWorker<NCGroup_result>* worker) {
			int ngrps;
			NC_CALL(nc_inq_grps(id, &ngrps, NULL));
			int *grp_ids = new int[ngrps];
			NC_CALL(nc_inq_grps(id, NULL, grp_ids));
			char name[NC_MAX_NAME + 1];
			for (int i = 0; i < ngrps; ++i) {
				int retval = nc_inq_grpname(grp_ids[i], name);
				if (retval == NC_NOERR && group_name==name) {
					NCGroup_result result;
					result.id = grp_ids[i];
					result.name = name;
					return result;
				}
			}
			throw std::runtime_error(string_format("NetCDF4: Bad or missing group \"%s\"",group_name.c_str()));
		},
		[] (Napi::Env env,NCGroup_result result) {
			return Group::Build(env, result.id,result.name);
		}
		
	);
	worker->Queue();

	return worker->Deferred().Promise(); 
}


Napi::Value Group::GetName(const Napi::CallbackInfo &info) {
//    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    Napi::Env env = info.Env();
    int id=this->id;
	Group * pGroup = this;
    auto worker=new NCAsyncWorker<NCGroup_result>(
		env,
		[id] (const NCAsyncWorker<NCGroup_result>* worker) {
			static NCGroup_result result;
            result.id = id;
            char name[NC_MAX_NAME + 1];
	        NC_CALL(nc_inq_grpname(id, name));
            result.name = std::string(name);
            return result;
		},
		[pGroup] (Napi::Env env, NCGroup_result result) mutable {
			pGroup->set_name(result.name);
         	return Napi::String::New(env, result.name);
		}
		
	);
	worker->Queue();
    return worker->Deferred().Promise();
}

Napi::Value Group::SetName(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    if (info.Length() < 1) {
        deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments"));
		return deferred.Promise();
	}
	std::string new_name = info[0].As<Napi::String>().Utf8Value();
    Napi::Env env = info.Env();
    int id=this->id;
	Group * pGroup = this;
    (new NCAsyncWorker<NCGroup_result>(
		env,
		deferred,
		[id,new_name] (const NCAsyncWorker<NCGroup_result>* worker) {
			static NCGroup_result result;
            result.id = id;
            result.name =  new_name;
            NC_CALL(nc_rename_grp(id, new_name.c_str()));
            return result;
		},
		[pGroup] (Napi::Env env, NCGroup_result result) mutable {
			pGroup->set_name(result.name);
         	return Napi::String::New(env, result.name);
		}
		
	))->Queue();
    return deferred.Promise();
}


Napi::Value Group::GetPath(const Napi::CallbackInfo &info) {
//    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    Napi::Env env = info.Env();
    int id=this->id;
	Group * pGroup = this;
    auto worker=new NCAsyncWorker<NCGroup_result>(
	    env,
	    [id,pGroup] (const NCAsyncWorker<NCGroup_result>* worker) {
		    static NCGroup_result result;
            result.id = id;
            size_t len;
            NC_CALL(nc_inq_grpname_len(result.id, &len));
	        char *name = new char[len + 1];
            name[len] = 0;
            NC_CALL(nc_inq_grpname_full(result.id, NULL, name));
            result.name = std::string(name);
            delete[] name;
            return result;
	    },
	    [pGroup] (Napi::Env env, NCGroup_result result) {
		    return Napi::String::New(env, result.name);
	   	}
	
	);
	worker->Queue();
    return worker->Deferred().Promise();
}

Napi::Value Group::Inspect(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), 
		string_format(
			"[Group %s]",
			this->name.c_str()
		)
	);
}

void Group::set_name(std::string groupname){
	this->name = name;
}
} // namespace netcdf4js
