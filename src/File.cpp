#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"

namespace netcdf4async {

struct File_result
{
	int id;
	int format;
};


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


	format=info[3].As<Napi::Number>().Int32Value();

//	Napi::Object group = info[3].As<Napi::Object>();
//	this->Value().Set("root", group);
//	closed=true;
}

File::File(
		const Napi::CallbackInfo &info,
		int id,std::string name,std::string mode,int format
	) : Napi::ObjectWrap<File>(info),id(id),name(name),mode(mode),format(format) {
	closed=true;
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
	return Napi::String::New(info.Env(), NC_FORMATS(format));
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
			NC_FORMATS(format),
			this->name.c_str()
		)
	);

}

Napi::Value File::Open(const Napi::CallbackInfo& info) {

	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (info.Length() < 2) {
		deferred.Reject(Napi::String::New(info.Env(), "Wrong number of arguments"));
		return deferred.Promise();
	}

	std::string name = info[0].As<Napi::String>().Utf8Value();
	std::string mode_arg = info[1].As<Napi::String>().Utf8Value();
	int open_format = NC_NETCDF4;
	int id=-1;
	Napi::Env env = info.Env();

	if (info.Length() > 2) {
		std::string format_arg = info[2].As<Napi::String>().Utf8Value();

		if (format_arg == "classic") {
			open_format = 0;
		} else if (format_arg == "classic64") {
			open_format = NC_64BIT_OFFSET;
		} else if (format_arg == "netcdf4") {
			open_format = NC_NETCDF4;
		} else if (format_arg == "netcdf4classic") {
			open_format = NC_NETCDF4 | NC_CLASSIC_MODEL;
		} else {
            deferred.Reject(Napi::String::New(info.Env(), "Unknown file format"));
    		return deferred.Promise();
		}
	}
	printf("Create\n");

		(new NCAsyncWorker<File_result>(
			env,
			deferred,
			[open_format,id,name,mode=mode_arg] (const NCAsyncWorker<File_result>* worker) {
				printf("Inside\n");
				printf("Try to open format %i\n",open_format);
				printf("Try to open %s,%s,%i\n",name.c_str(),mode.c_str(),open_format);
				File_result result;

				if (mode == "r") {
					printf("r\n");
					NC_CALL(nc_open(name.c_str(), NC_NOWRITE, &result.id));
				} else if (mode == "w") {
					printf("w\n");
					NC_CALL(nc_open(name.c_str(), NC_WRITE, &result.id));
				} else if (mode == "c") {
					printf("c\n");
					NC_CALL(nc_create(name.c_str(), open_format | NC_NOCLOBBER, &result.id));
				} else if (mode == "c!") {
					NC_CALL(nc_create(name.c_str(), open_format | NC_CLOBBER, &result.id));
				} else {
					throw std::runtime_error("NetCDF4: Unknown file mode");
				}
				printf("Opened");
				NC_CALL(nc_inq_format_extended(id,&result.format,NULL));
				return result;
				// this->format=i;
				
				
			},
			[name,mode_arg] (Napi::Env env,File_result result) mutable {
				printf("Deferred");
				return File::Build(env,result.id,name,mode_arg,result.format);
			//	deferred.Resolve();
			}
			
		))->Queue();
	printf("Created\n");

//		printf("Run queue\n");
//		worker->Queue();


	return deferred.Promise();
}


} // namespace netcdf4async 
