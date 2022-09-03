#include <napi.h>
#include <netcdf_meta.h>
#include "netcdf4-async.h"


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
	Napi::Env env = info.Env();
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (info.Length() < 2) {
		deferred.Reject(Napi::String::New(info.Env(), "Wrong number of arguments"));
		return deferred.Promise();
	}

	deferred.Resolve(
		File::Build(env,
			-1,
			info[0].As<Napi::String>().Utf8Value(),
			info[1].As<Napi::String>().Utf8Value(),
			2
			)
	);
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
