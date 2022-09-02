#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"

namespace netcdf4async {

Napi::FunctionReference File::constructor;

File::~File() {
	if (!closed) {
		nc_close(id);
	}
}

Napi::Object File::Build(Napi::Env env,int ncid,const std::string name,const std::string mode,int type) {
	return constructor.New({
		Napi::Number::New(env,ncid),
		Napi::String::New(env,name.c_str()),
		Napi::String::New(env,mode.c_str()),
		Napi::Number::New(env,type)
	});
}

void File::Init(Napi::Env env) {
	Napi::HandleScope scope(env);

	Napi::Function func =
		DefineClass(env, "File",
			{
				InstanceMethod("sync", &File::Sync), 
				InstanceMethod("close", &File::Close),
				InstanceMethod("dataMode", &File::DataMode),
				InstanceAccessor<&File::GetName>("name"),
				InstanceAccessor<&File::IsClosed>("closed"),
				InstanceAccessor<&File::GetFormat>("format"),
				InstanceMethod("inspect", &File::Inspect)
			}
		);

	constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

//	exports.Set("File", func);
//	return exports;
}

File::File(const Napi::CallbackInfo &info) : Napi::ObjectWrap<File>(info) {

	if (info.Length() < 4) {
		Napi::TypeError::New(info.Env(), "Wrong number of arguments").ThrowAsJavaScriptException();
		return;
	}

	id = info[0].As<Napi::Number>().Int32Value();
	
	name = info[1].As<Napi::String>().Utf8Value();

	mode = info[2].As<Napi::String>().Utf8Value();

	format=NC_FORMATS(info[3].As<Napi::Number>().Int32Value());

//	Napi::Object group = info[3].As<Napi::Object>();
//	this->Value().Set("root", group);
	closed=false;
}

Napi::Value File::Sync(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (!this->closed) {
		deferred.Reject(Napi::String::New(info.Env(), "Not implemented yet"));
	}
	else {
		deferred.Reject(Napi::String::New(info.Env(), "File already closed"));
	}
	return deferred.Promise();
}

Napi::Value File::Close(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (!this->closed) {
		deferred.Reject(Napi::String::New(info.Env(), "Not implemented yet"));
	}
	else {
		deferred.Resolve(Napi::String::New(info.Env(),"File already closed"));
	}
	return deferred.Promise();
}

Napi::Value File::GetName(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), name);
}

Napi::Value File::IsClosed(const Napi::CallbackInfo &info) {
	return Napi::Boolean::New(info.Env(), closed);
}

Napi::Value File::GetFormat(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), format);
}

Napi::Value File::DataMode(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (!this->closed) {
		deferred.Reject(Napi::String::New(info.Env(), "Not implemented yet"));
	}
	else {
		deferred.Reject(Napi::String::New(info.Env(), "File closed"));
	}
	return deferred.Promise();
}

Napi::Value File::Inspect(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), 
		string_format(
			"[%s%s file %s]",
			this->closed?"Closed ":"",
			this->format.c_str(),
			this->name.c_str()
		)
	);

}
} // namespace netcdf4async 
