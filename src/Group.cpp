#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"
#include "Attribute.h"

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

struct VariableInfo{
	int var_id;
	int parent_id;
	std::string name;
	nc_type type;
	int ndims;
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
		        InstanceMethod("getVariable",&Group::GetVariable),
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
	if (info.Length() != static_cast<size_t>(3)) {
		deferred.Reject(Napi::String::New(info.Env(),"Not all parameters from name,type,value bound"));
		return deferred.Promise();
	}
	std::string type_str=info[1].As<Napi::String>().ToString();
	int type=get_type(type_str);
	std::string name=info[0].As<Napi::String>().ToString();
	Napi::Env env = info.Env();
	int id = this->id;
	return add_attribute(env, deferred, id, NC_GLOBAL, name, type, info[2]);
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
	struct VariableInfo {
		int id;
		int parent_id;
		std::string name;
		nc_type type;
		int ndims;
	};
	

	Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    if (info.Length() != static_cast<size_t>(3)) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments. Need variable name, type and dimenisons"));
		return deferred.Promise();
	}
	if (!info[2].IsArray()) {
		deferred.Reject(Napi::String::New(info.Env(),"Dimensions must be an array"));
		return deferred.Promise();
	}
	auto dims=info[2].As<Napi::Array>();
	auto dims_size=dims.Length();
	if (dims_size==0u) {
		deferred.Reject(Napi::String::New(info.Env(),"Dimensions must be a non-empty array"));
		return deferred.Promise();
	}
	std::string type_str=info[1].As<Napi::String>().ToString();
	int type=get_type(type_str);
	if (type==NC2_ERR) {
		deferred.Reject(Napi::String::New(info.Env(),"Bad variable type"));
		return deferred.Promise();
	}

	std::string name=info[0].As<Napi::String>().ToString();
	std::vector<std::unique_ptr<const std::string > >* dim_name = new std::vector<std::unique_ptr<const std::string > >() ;
	for(auto i=0u;i<dims_size;i++) {
		Napi::Value dm=dims[i];
		dim_name->push_back(std::make_unique<std::string>(dm.As<Napi::String>().Utf8Value()));
	}

	auto worker=new NCAsyncWorker<VariableInfo>(
		env,
		deferred,
		[id=this->id, name, type, dims_size, dim_name] (const NCAsyncWorker<VariableInfo>* worker) {
			int *ndims = new int[dims_size];
			for(auto i=0u;i<dims_size;i++) {
				NC_CALL(nc_inq_dimid(id, dim_name->at(i)->c_str(), &ndims[i]));
			}
			int new_id;
			NC_CALL(nc_def_var(id,name.c_str(), type, dims_size, ndims ,&new_id));
			delete[] ndims;
			dim_name->clear();
			dim_name->~vector();
			VariableInfo result;
			result.id= new_id;
			result.parent_id = id;
			result.name = name;
			result.ndims = dims_size;
			result.type = type;
			return result;
		},
		[] (Napi::Env env,VariableInfo result) {
			
			return Variable::Build(env,result.id, result.parent_id, result.name, result.type, result.ndims);
		}

	);
	worker->Queue();

	return worker->Deferred().Promise(); 
}

Napi::Value Group::GetId(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), id);
}

Napi::Value Group::GetVariables(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

	auto worker=new NCAsyncWorker<std::vector<VariableInfo>>(
		env,
		[parent_id=this->id] (const NCAsyncWorker<std::vector<VariableInfo>>* worker) {
			int nvars;
			NC_CALL(nc_inq_varids(parent_id, &nvars, NULL));
			int *var_ids = new int[nvars];
			NC_CALL(nc_inq_varids(parent_id, NULL, var_ids));
			std::vector<VariableInfo> variables;
			for(int i=0; i<nvars; i++){
				VariableInfo varInfo;
				varInfo.var_id = var_ids[i];
				varInfo.parent_id = parent_id;
				char varName[NC_MAX_NAME + 1];
				NC_CALL(nc_inq_var(varInfo.parent_id, varInfo.var_id, varName, &varInfo.type, &varInfo.ndims, NULL, NULL));
				varInfo.name = std::string(varName);
				variables.push_back(varInfo);
			}
			delete[] var_ids;
			return variables;
		},
		[] (Napi::Env env,std::vector<VariableInfo> result) {
			Napi::Object vars = Napi::Object::New(env);	
			for (auto var = result.begin(); var < result.end(); ++var) {
				Napi::Object varObj = Variable::Build(env, var->var_id,
					var->parent_id, var->name, var->type, var->ndims);
		 		vars.Set(var->name, varObj);
	 		}
			return vars;
		}
	);
	worker->Queue();
	
    return worker->Deferred().Promise();
}

Napi::Value Group::GetVariable(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	Napi::Env env = info.Env();

	if (info.Length() != static_cast<size_t>(1)) {
	 	deferred.Reject(Napi::String::New(env, "Wrong number of arguments. Missed variable name"));
	    return deferred.Promise();
	}
	
	const std::string var_name=info[0].As<Napi::String>().ToString();

	auto worker=new NCAsyncWorker<VariableInfo>(
		env,
		deferred,
		[parent_id=this->id,var_name] (const NCAsyncWorker<VariableInfo>* worker) {
			int nvars;
			NC_CALL(nc_inq_varids(parent_id, &nvars, NULL));
			int *var_ids = new int[nvars];
			NC_CALL(nc_inq_varids(parent_id, NULL, var_ids));
			VariableInfo varInfo;
			char varName[NC_MAX_NAME + 1];
			varInfo.parent_id = parent_id;
			for(int i=0; i<nvars; i++){
				varInfo.var_id = var_ids[i];
				NC_CALL(nc_inq_var(varInfo.parent_id, varInfo.var_id, varName, &varInfo.type, &varInfo.ndims, NULL, NULL));
				if (var_name == varName) {
					varInfo.name = std::string(varName);
					delete[] var_ids;
					return varInfo;
				}
			}
			delete[] var_ids;
			throw std::runtime_error(string_format("NetCDF4: Variable %s not found",var_name.c_str()));
		},
		[] (Napi::Env env,VariableInfo result) {
			Napi::Object varObj = Variable::Build(env, result.var_id,
				result.parent_id, result.name, result.type, result.ndims);
			return varObj;
		}
	);
	worker->Queue();


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
 	return netcdf4async::get_attributes(env, id, NC_GLOBAL, return_type).Promise();
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
