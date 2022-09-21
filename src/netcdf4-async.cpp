#include <napi.h>
#include <node_api.h>
#include <netcdf_meta.h>
#include <memory>
#include "netcdf4-async.h"
// #include "worker.h"


namespace netcdf4async {
    /// @brief List of variable/attribute types
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
    /// @brief List of file format names
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

/**
 * @brief Async open file implementation
 * 
 * @param info 
 * @return Napi::Value Returns Promise object
 */
Napi::Value open(const Napi::CallbackInfo& info) {


    return File::Open(info);
/*
    void* native;
    napi_unwrap(info.Env(),file_obj,&native);
    std::unique_ptr<File> file=std::unique_ptr<File>(static_cast<File *>(native));
    file->openAsync(info,deferred);
*/    

}

/**
 * @brief Initialize class as NodeJS object
 * 
 * @param env nodejs environment
 * @param exports Object contains module exports
 * @return Napi::Object Object contains module exports
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {
	Napi::Object version=Napi::Object::New(env);
	version.Set("major",NC_VERSION_MAJOR);
	version.Set("minor",NC_VERSION_MINOR);
	version.Set("patch",NC_VERSION_PATCH);
	version.Set("version",NC_VERSION);
	exports.Set("version",version);

	exports.Set(Napi::String::New(env, "open"), Napi::Function::New<open>(env));	
	File::Init(env);
	Group::Init(env);
//	Dimension::Init(env, exports);
//	Attribute::Init(env, exports);
	Variable::Init(env);
	return exports;
}

NODE_API_MODULE(hello, Init)
}