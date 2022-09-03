#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4js.h"
#include "netcdf4jstypes.h"
#include "NetCDFFile.h"

namespace netcdf4js {

Napi::FunctionReference File::constructor;

File::~File() {
	if (!closed) {
		NC_CALL_ENV(this->Env(), nc_close(id));
	}
}

Napi::Object File::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);

	Napi::Function func =
		DefineClass(env, "File",
			{
				InstanceMethod("sync", &File::Sync), 
				InstanceMethod("close", &File::Close),
				InstanceMethod("dataMode", &File::DataMode),
				InstanceAccessor<&File::GetId>("id"),
				InstanceAccessor<&File::GetName>("name"),
				InstanceAccessor<&File::IsClosed>("closed"),
				InstanceAccessor<&File::GetFormat>("format"),
				InstanceMethod("inspect", &File::Inspect)
			}
		);

	constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

	exports.Set("File", func);
	return exports;
}

File::File(const Napi::CallbackInfo &info, NetCDFFile netcdf_file ) : Napi::ObjectWrap<NetCDFFile>(info), file(netcdf_file) {
	// auto group = Group::Build(info.Env(), id);
	// this->Value().Set("root", group);
	// format=NC_FORMATS(open_format);
	// closed=false;
}

Napi::Value File::Sync(const Napi::CallbackInfo &info) {
	if (!this->closed) {
		Napi::Env env = info.Env();

		int retval = nc_sync(this->id);
		if (retval != NC_NOERR) {
			Napi::Error::New(env, nc_strerror(retval)).ThrowAsJavaScriptException();
		}
	}

	return info.Env().Undefined();
}

Napi::Value File::Close(const Napi::CallbackInfo &info) {
	if (!this->closed) {
		this->closed = true;
		int retval = nc_close(this->id);
		if (retval != NC_NOERR && retval != NC_EBADID) {
			Napi::Error::New(info.Env(), nc_strerror(retval)).ThrowAsJavaScriptException();
		}
		if (this->Value().Has("root")) {
			this->Value().Delete("root");
		}		
	}
	return info.Env().Undefined();
}

Napi::Value File::GetId(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), id);
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
	int retval = nc_enddef(this->id);
	if (retval != NC_NOERR) {
		Napi::Error::New(info.Env(), nc_strerror(retval)).ThrowAsJavaScriptException();
	}
	return info.Env().Undefined();

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
} // namespace netcdf4js
