#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"
#include "Attribute.h"

namespace netcdf4async {

/*
const unsigned char Variable::type_sizes[] = {
	0, // NC_NAT // unknown type
	1, // NC_BYTE
	1, // NC_CHAR
	2, // NC_SHORT
	4, // NC_INT / NC_LONG
	4, // NC_FLOAT
	8, // NC_DOUBLE
	1, // NC_UBYTE
	2, // NC_USHORT
	4  // NC_UINT
};
*/


Napi::FunctionReference Variable::constructor;

Napi::Object Variable::Build(Napi::Env env, int id, int parent_id) {
	return constructor.New({Napi::Number::New(env, id), Napi::Number::New(env, parent_id)});
}

Variable::Variable(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Variable>(info) {
	// if (info.Length() < 2) {
	// 	Napi::TypeError::New(info.Env(), "Wrong number of arguments").ThrowAsJavaScriptException();
	// 	return;
	// }

	// char varName[NC_MAX_NAME + 1];
	// id = info[0].As<Napi::Number>().Int32Value();
	// parent_id = info[1].As<Napi::Number>().Int32Value();
	// NC_CALL_VOID(nc_inq_var(parent_id, id, varName, &type, &ndims, NULL, NULL));
	// name = std::string(varName);
}

void Variable::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
    Napi::Function func =
		DefineClass(env, "Variable",
			{
                InstanceMethod("read", &Variable::Read),
		        InstanceMethod("readSlice", &Variable::ReadSlice),
		        InstanceMethod("readStridedSlice", &Variable::ReadStridedSlice),
		        InstanceMethod("write", &Variable::Write),
		        InstanceMethod("writeSlice", &Variable::WriteSlice),
		        InstanceMethod("writeStridedSlice", &Variable::WriteStridedSlice),
		        InstanceMethod("addAttribute", &Variable::AddAttribute),
		        InstanceMethod("inspect", &Variable::Inspect),

                InstanceMethod("getName", &Variable::GetName),
                InstanceMethod("setName", &Variable::SetName),
                InstanceMethod("getDimensions", &Variable::GetDimensions),
                InstanceMethod("getFill", &Variable::GetFill),
                InstanceMethod("setFill", &Variable::SetFill),
                InstanceMethod("getChunked", &Variable::GetChunked),
                InstanceMethod("setChunked", &Variable::SetChunked),
                InstanceMethod("getDeflateInfo", &Variable::GetDeflateInfo),
                InstanceMethod("setDeflateInfo", &Variable::SetDeflateInfo),
                InstanceMethod("getEndiannes", &Variable::GetEndiannes),
                InstanceMethod("setEndiannes", &Variable::SetEndiannes),
                InstanceMethod("getChecksumMode", &Variable::GetChecksumMode),
                InstanceMethod("setChecksumMode", &Variable::SetChecksumMode),
                InstanceMethod("getAttributes", &Variable::GetAttributes),
                InstanceMethod("setAttribute", &Variable::SetAttribute),
                InstanceMethod("addAttribute", &Variable::AddAttribute),
                InstanceMethod("renameAttribute", &Variable::RenameAttribute),
                InstanceMethod("deleteAttribute", &Variable::DeleteAttribute),

		        InstanceAccessor<&Variable::GetTypeSync>("type"),
		        InstanceAccessor<&Variable::GetNameSync>("name"),
		        
			}
		);
    constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

}

Napi::Value Variable::GetTypeSync(const Napi::CallbackInfo &info) {
	const char *res=NC_TYPES(this->type);
	return Napi::String::New(info.Env(),res);

}

Napi::Value Variable::GetName(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();	
}

Napi::Value Variable::GetNameSync(const Napi::CallbackInfo &info) {
    return Napi::String::New(info.Env(),this->name.c_str());
}


Napi::Value  Variable::SetName(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetDimensions(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}


Napi::Value Variable::GetFill(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetFill(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetChunked(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetChunked(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetDeflateInfo(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetDeflateInfo(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetEndiannes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::SetEndiannes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetChecksumMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetChecksumMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetAttributes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::AddAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}


Napi::Value Variable::SetAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::RenameAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::DeleteAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::Write(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}


Napi::Value Variable::WriteSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::WriteStridedSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();

}


Napi::Value Variable::Read(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
	
}


Napi::Value Variable::ReadSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();

}

Napi::Value Variable::ReadStridedSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();

}

Napi::Value Variable::Inspect(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), 
		string_format(
			"[Variable %s, type %s, %i dimension(s)]",
			this->name.c_str(),
			NC_TYPES(this->type),
			this->ndims)
	);
}


}// namespace netcdf4async