#include <napi.h>
#include <netcdf_meta.h>
#include "netcdf4-async.h"
#include "worker.h"


namespace netcdf4async {

    const char *type_names[] = {
        "unknown", // NC_NAT // unknown type
        "byte",	   // NC_BYTE
        "char",	   // NC_CHAR
        "short",   // NC_SHORT
        "int",	   // NC_INT / NC_LONG
        "float",   // NC_FLOAT
        "double",  // NC_DOUBLE
        "ubyte",   // NC_UBYTE
        "ushort",  // NC_USHORT
        "uint",	   // NC_UINT
        "int64",	   // NC_INT64
        "uint64",	   // NC_UINT64
        "string"
    };
    const char *format_names[] = {
        "unknown", // NC_FORMATX_UNDEFINED
        "netcdf3",	   // NC_FORMATX_NC3
        "hdf5",	   // NC_FORMATX_HDF5
        "hdf4",   // NC_FORMATX_HDF4 / NC_FORMATX_NC4
        "pnetcdf", // NC_FORMATX_PNETCDF
        "dap2",   // NC_FORMATX_DAP2
        "dap4",  // NC_FORMATX_DAP4
        "?",   // 
        "udf0",  // NC_FORMATX_UDF0
        "udf1",	   // NC_FORMATX_UDF1
        "zarr"	   // NC_FORMATX_NCZARR
    };


}
using namespace netcdf4async;

Napi::Value open(const Napi::CallbackInfo& info) {
	printf("Start openFile\n");
	Napi::Env env = info.Env();
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());

	if (info.Length() < 2) {
		deferred.Reject(Napi::String::New(info.Env(), "Wrong number of arguments"));
		return deferred.Promise();
	}
    std::string name = info[0].As<Napi::String>().Utf8Value();

	std::string mode_arg = info[1].As<Napi::String>().Utf8Value();

	int open_format = NC_NETCDF4;
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
	printf("Create Async Worker\n");

    NetCDFPromiseWorker<int> * onenFileWorker = new NetCDFPromiseWorker<int>(info.Env(), &deferred, 
        [name, mode_arg, open_format](){
            int id;
            if (mode_arg == "r") {
		        NC_CALL(nc_open(name.c_str(), NC_NOWRITE, &id));
	        } else if (mode_arg == "w") {
		        NC_CALL(nc_open(name.c_str(), NC_WRITE, &id));
	        } else if (mode_arg == "c") {
		        NC_CALL(nc_create(name.c_str(), open_format | NC_NOCLOBBER, &id));
	        } else if (mode_arg == "c!") {
		        NC_CALL(nc_create(name.c_str(), open_format | NC_CLOBBER, &id));
	        } else {
		        throw std::runtime_error("Unknown file mode");
	        }
	        NC_CALL(nc_inq_format_extended(id, const_cast<int*>(&open_format),NULL));
            return id;
        },
        [env, name, mode_arg, open_format](int id){
            return File::Build(env,
				id,
				name,
				mode_arg,
				open_format
			);

        }); 

	onenFileWorker->Queue();

	return deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	Napi::Object version=Napi::Object::New(env);
	version.Set("major",NC_VERSION_MAJOR);
	version.Set("minor",NC_VERSION_MINOR);
	version.Set("patch",NC_VERSION_PATCH);
	version.Set("version",NC_VERSION);
	exports.Set("version",version);

	exports.Set(Napi::String::New(env, "open"), Napi::Function::New<open>(env));	
	File::Init(env);
//	Group::Init(env, exports);
//	Dimension::Init(env, exports);
//	Attribute::Init(env, exports);
//	Variable::Init(env, exports);
	return exports;
}

NODE_API_MODULE(hello, Init)
