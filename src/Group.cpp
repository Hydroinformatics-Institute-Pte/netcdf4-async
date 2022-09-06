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

Napi::FunctionReference Group::constructor;

Napi::Object Group::Build(Napi::Env env, int id) {
	return constructor.New({Napi::Number::New(env, id)});
}

Group::Group(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Group>(info) {
	if (info.Length() < 1) {
		Napi::TypeError::New(info.Env(), "Wrong number of arguments").ThrowAsJavaScriptException();
		return;
	}

	id = info[0].As<Napi::Number>().Int32Value();
}

Group::Group(const Napi::CallbackInfo &info, int id, std::string name):
    Napi::ObjectWrap<Group>(info), id(id), name(name) {}

void Group::Init(Napi::Env env) {

	Napi::HandleScope scope(env);

    Napi::Function func =
		DefineClass(env, "Group",
			{
				InstanceMethod("addVariable", &Group::AddVariable),
		        InstanceMethod("addDimension", &Group::AddDimension),
		        InstanceMethod("addSubgroup", &Group::AddSubgroup),
		        InstanceMethod("addAttribute", &Group::AddAttribute),
		        InstanceMethod("inspect", &Group::Inspect),
                InstanceMethod("getName", &Group::GetName),
                InstanceMethod("setName", &Group::SetName),
                InstanceAccessor<&Group::GetId>("id"),
		        InstanceAccessor<&Group::GetVariables>("variables"),
		        InstanceAccessor<&Group::GetDimensions>("dimensions"),
		        InstanceAccessor<&Group::GetUnlimited>("unlimited"),
		        InstanceAccessor<&Group::GetAttributes>("attributes"),
		        InstanceAccessor<&Group::GetSubgroups>("subgroups"),
		        InstanceAccessor<&Group::GetFullname>("fullname"),
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
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	// if (info.Length() != static_cast<size_t>(1)) {
	// 	Napi::TypeError::New(info.Env(), "Missing subgroup name").ThrowAsJavaScriptException();
	// 	return info.Env().Undefined();
	// }
	// std::string new_name = info[0].As<Napi::String>().ToString();
	// int new_id;
	// NC_CALL(nc_def_grp(id, new_name.c_str(),&new_id));
	// Napi::Object group = Group::Build(info.Env(), new_id);
	// return group;
	return deferred.Promise(); 
}

Napi::Value Group::AddDimension(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	// if (info.Length() != static_cast<size_t>(2)) {
	// 	Napi::TypeError::New(info.Env(), "Wrong number of arguments. Need dimension name and length").ThrowAsJavaScriptException();
	// 	return info.Env().Undefined();
	// }
	// int len;
	// const std::string len_symbol=info[1].As<Napi::String>().ToString();
	// if (len_symbol=="unlimited") {
	// 	len=NC_UNLIMITED;
	// }
	// else {
	// 	len=info[1].As<Napi::Number>().Int32Value();
	// 	if (len<=0) {
	// 		Napi::TypeError::New(info.Env(), "Expected positive integer as dimension length").ThrowAsJavaScriptException();
	// 		return info.Env().Undefined();
	// 	}
	// }
	// const std::string new_name = info[0].As<Napi::String>().ToString();
	// int new_id;
	// NC_CALL(nc_def_dim(id,new_name.c_str(),len,&new_id))
	// Napi::Object dimension = Dimension::Build(info.Env(),id,new_id);
	// return dimension;
    return deferred.Promise(); 
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
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	// int ndims;
	// NC_CALL(nc_inq_dimids(this->id, &ndims, NULL, 0));
	// int *dim_ids = new int[ndims];
	// NC_CALL(nc_inq_dimids(this->id, NULL, dim_ids, 0));

	// Napi::Object dims = Napi::Object::New(info.Env());

	// char name[NC_MAX_NAME + 1];
	// for (int i = 0; i < ndims; ++i) {
	// 	Napi::Object dim = Dimension::Build(info.Env(), this->id, dim_ids[i]);

	// 	int retval = nc_inq_dimname(this->id, dim_ids[i], name);
	// 	if (retval == NC_NOERR) {
	// 		dims.Set(name, dim);
	// 	}
	// }
	// delete[] dim_ids;
	// return dims;
    return deferred.Promise();
}

Napi::Value Group::GetUnlimited(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	
	// int ndims;
	// NC_CALL(nc_inq_unlimdims(this->id, &ndims, NULL));
	
	// int *dim_ids = new int[ndims];
	// NC_CALL(nc_inq_unlimdims(this->id, NULL, dim_ids));
	
	// Napi::Object dims = Napi::Object::New(info.Env());
    // char name[NC_MAX_NAME + 1];
	// for (int i = 0; i < ndims; ++i) {
	// 	Napi::Object dim = Dimension::Build(info.Env(), this->id, dim_ids[i]);
	// 	NC_CALL(nc_inq_dimname(this->id, dim_ids[i], name));
	// 	dims.Set(name, dim);
	// }
	
	// delete[] dim_ids;
	// return dims;
    return deferred.Promise();
}

Napi::Value Group::GetAttributes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	// int natts;
	// NC_CALL(nc_inq_natts(this->id, &natts));

	// Napi::Object attrs = Napi::Object::New(info.Env());
	// char name[NC_MAX_NAME + 1];
	// for (int i = 0; i < natts; ++i) {
	// 	NC_CALL(nc_inq_attname(this->id, NC_GLOBAL, i, name));
	// 	Napi::Object attr = Attribute::Build(info.Env(), name, NC_GLOBAL, this->id);
	// 	attrs.Set(name, attr);
	// }
	// return attrs;
    return deferred.Promise();
}

Napi::Value Group::GetSubgroups(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
	// int ngrps;
	// NC_CALL(nc_inq_grps(this->id, &ngrps, NULL));
	// int *grp_ids = new int[ngrps];

	// NC_CALL(nc_inq_grps(this->id, NULL, grp_ids));

	// Napi::Object subgroups = Napi::Object::New(info.Env());

	// char name[NC_MAX_NAME + 1];
	// for (int i = 0; i < ngrps; ++i) {
	// 	Napi::Object subgroup = Group::Build(info.Env(), grp_ids[i]);
	// 	int retval = nc_inq_grpname(grp_ids[i], name);
	// 	if (retval == NC_NOERR) {
	// 		subgroups.Set(name, subgroup);
	// 	}
	// }

	// delete[] grp_ids;
	// return subgroups;
	return deferred.Promise(); 
}

Napi::Value Group::GetName(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    Napi::Env env = info.Env();
    int id=this->id;
    (new NCAsyncWorker<NCGroup_result>(
		env,
		deferred,
		[id] (const NCAsyncWorker<NCGroup_result>* worker) {
			static NCGroup_result result;
            result.id = id;
            char name[NC_MAX_NAME + 1];
	        NC_CALL(nc_inq_grpname(id, name));
            result.name = std::string(name);
            return result;
		},
		[] (Napi::Env env, NCGroup_result result) mutable {
         	return Napi::String::New(env, result.name);
		}
		
	))->Queue();
    return deferred.Promise();
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
		[] (Napi::Env env, NCGroup_result result) mutable {
         	return Napi::String::New(env, result.name);
		}
		
	))->Queue();
    return deferred.Promise();
}


Napi::Value Group::GetFullname(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    Napi::Env env = info.Env();
    int id=this->id;
    (new NCAsyncWorker<NCGroup_result>(
	    env,
	    deferred,
	    [id] (const NCAsyncWorker<NCGroup_result>* worker) {
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
	    [] (Napi::Env env, NCGroup_result result) {
		    return Napi::String::New(env, result.name);
	   	}
	
	))->Queue();
    return deferred.Promise();
}

Napi::Value Group::Inspect(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), 
		string_format(
			"[Group %s]",
			this->name.c_str()
		)
	);
}
} // namespace netcdf4js
