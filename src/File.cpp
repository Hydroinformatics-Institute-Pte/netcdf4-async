#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"

namespace netcdf4async {

/**
 * @brief File operation results
 * 
 */
struct NCFile_result
{
	/// @brief File id
	int id;
	/// @brief File format
	int format;
};


Napi::FunctionReference File::constructor;

/**
 * @brief Destroy the File:: File object
 * Destroy file and close netcdf file (if not closed yet)
 */
File::~File() {
	if (!closed) {
		nc_close(id);
	}
}

/**
 * @brief Build Node file object
 * Create object which handles NetCDF file
 * NB! File must be opened before create
 * @param env NodeJS environment
 * @param ncid NetCDF file id
 * @param name file full path
 * @param mode file mode
 * @param type file type
 * @return Napi::Object 
 */
Napi::Object File::Build(Napi::Env env,int ncid,const std::string name,const std::string mode,int type) {
	return constructor.New({
		Napi::Number::New(env,ncid),
		Napi::String::New(env,name.c_str()),
		Napi::String::New(env,mode.c_str()),
		Napi::Number::New(env,type)
	});
}

/**
 * @brief Define NodeJS object description 
 * 
 * @param env NodeJS env
 */
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

/**
 * @brief Construct a new File:: File object
 * Construct NodeJS wrapper for netcdf file.
 * @param info NodeJS parameters
 */
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

/**
 * @brief Construct a new File:: File object
 * Construct NodeJS wrapper for netcdf file.
 * @param info NodeJS params
 * @param id file id
 * @param name file path
 * @param mode file mode
 * @param format file format
 */
File::File(
		const Napi::CallbackInfo &info,
		int id,std::string name,std::string mode,int format
	) : Napi::ObjectWrap<File>(info),id(id),name(name),mode(mode),format(format) {
	closed=true;
}

/**
 * @brief Async open netCDF file
 * Either create new File object __after__ open file, or rejects 
 * @param info NodeJS params
 * @return Napi::Value Deferred promise object
 */
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
            deferred.Reject(Napi::String::New(info.Env(), "NetCDF4: Unknown file format"));
    		return deferred.Promise();
		}
	}

	int mode=0;
	bool create=false;
	if (mode_arg == "r") {
		mode=NC_NOWRITE;
	} else if (mode_arg == "w") {
		mode=NC_WRITE;
	} else if (mode_arg == "c") {
		mode=open_format | NC_NOCLOBBER;
		create=true;
	} else if (mode_arg == "c!") {
		mode=open_format | NC_CLOBBER;
		create=true;
	} else {
		deferred.Reject(Napi::String::New(info.Env(), "NetCDF4: Unknown file mode"));
		return deferred.Promise();
	}


	(new NCAsyncWorker<NCFile_result>(
		env,
		deferred,
		[open_format,id,name,mode,create] (const NCAsyncWorker<NCFile_result>* worker) {
			static NCFile_result result;
			if (create) {
				NC_CALL(nc_create(name.c_str(), mode, &result.id));
			}
			else {
				NC_CALL(nc_open(name.c_str(), mode, &result.id));
			}
			NC_CALL(nc_inq_format_extended(result.id,&result.format,NULL));
			return result;
			// this->format=i;
		},
		[name,mode_arg] (Napi::Env env,NCFile_result result)  {
			return File::Build(env,result.id,name,mode_arg,result.format);
		}
		
	))->Queue();

	return deferred.Promise();
}

/**
 * @brief Async switch file from definition mode
 * 
 * @param info NodeJS params
 * @return Napi::Value Deferred promise object
 */
Napi::Value File::DataMode(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (!this->closed) {
		int id=this->id;
		(new NCAsyncWorker<NCFile_result>(
			env,
			deferred,
			[id] (const NCAsyncWorker<NCFile_result>* worker) {
				static NCFile_result result;
				result.id=id;
		        NC_CALL(nc_enddef(id))
				return result;
				// this->format=i;
			},
			[] (Napi::Env env,NCFile_result result)  {
				return Napi::Number::New(env,result.id);
			//	deferred.Resolve();
			}
			
		))->Queue();
	}
	else {
		deferred.Reject(Napi::String::New(info.Env(), "File already closed"));
	}
	return deferred.Promise();
}

/**
 * @brief Perform netCDF file sync. 
 * Synchronize data from memory to disk and vice verse.
 * @param info 
 * @return Napi::Value Deferred promise object
 */
Napi::Value File::Sync(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (!this->closed) {
		int id=this->id;
		(new NCAsyncWorker<NCFile_result>(
			env,
			deferred,
			[id] (const NCAsyncWorker<NCFile_result>* worker) {
				static NCFile_result result;
				result.id=id;
		        NC_CALL(nc_sync(id))
				return result;
				// this->format=i;
			},
			[] (Napi::Env env,NCFile_result result)  {
				return Napi::Number::New(env,result.id);
			//	deferred.Resolve();
			}
			
		))->Queue();
	}
	else {
		deferred.Reject(Napi::String::New(info.Env(), "File already closed"));
	}
	return deferred.Promise();
}

/**
 * @brief Closes netCDF file
 * Closes netcdf file and removes root variable group
 * @param info 
 * @return Napi::Value Deferred promise object
 */
Napi::Value File::Close(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(env);
	if (!this->closed) {
		this->closed=true;
		int id=this->id;
		(new NCAsyncWorker<NCFile_result>(
			env,
			deferred,
			[id] (const NCAsyncWorker<NCFile_result>* worker) {
				static NCFile_result result;
				result.id=id;
		        NC_VOID_CALL(nc_close(id))
				return result;
				// this->format=i;
			},
			[] (Napi::Env env,NCFile_result result)  {
				return Napi::Number::New(env,result.id);
			//	deferred.Resolve();
			}
			
		))->Queue();

		
	}
	else {
		deferred.Resolve(Napi::String::New(info.Env(),"File already closed"));
	}
	return deferred.Promise();
}

/**
 * @brief Return full file path
 * 
 * @param info 
 * @return Napi::Value NodeJS string representation of file path
 */
Napi::Value File::GetName(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), name);
}

/**
 * @brief is file closed?
 * 
 * @param info 
 * @return Napi::Value NodeJS boolean representation, =true if file closed
 */
Napi::Value File::IsClosed(const Napi::CallbackInfo &info) {
	return Napi::Boolean::New(info.Env(), closed);
}

/**
 * @brief Return file format
 * 
 * @param info 
 * @return Napi::Value 
 */
Napi::Value File::GetFormat(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), NC_FORMATS(format));
}

/**
 * @brief Return string representation for file object
 * Basicly use in toString() method of NodeJS object
 * @param info 
 * @return Napi::Value 
 */
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


} // namespace netcdf4async 

