#ifndef NETCDF4ASYNC_H
#define NETCDF4ASYNC_H

#include <napi.h>
#include <netcdf.h>
#include <string>
#include <memory>
#include <stdexcept>

#define NC_CALL(FN)                                                                                \
	do {                                                                                           \
		int retval = FN;                                                                           \
		if (retval != NC_NOERR) {                                                                  \
			throw std::runtime_error("NetCDF4: " + std::string(nc_strerror(retval)));              \
		}                                                                                          \
	} while (false);

#if NODE_MAJOR_VERSION > 9
#define NC_TYPES(ID) ID < NC_BYTE || (ID > NC_UINT64 && ID != NC_STRING)?"unsupported":netcdf4async::type_names[ID]
#else
#define NC_TYPES(ID) ID < NC_BYTE || (ID > NC_UINT && ID != NC_STRING)?"unsupported":netcdf4async::type_names[ID]
#endif

// Ubuntu 22.04 and Ubuntu 20.04 have a different name for NCZARR file type constant
// So....
#if NETCDF_VERSION_MINOR < 7
#define NC_FORMATS(ID) ID < NC_FORMATX_UNDEFINED || ID > NC_FORMATX_DAP4 ?"unsupported":netcdf4async::format_names[ID]
#endif

#if NETCDF_VERSION_MINOR < 8 && NETCDF_VERSION_MINOR >7
#ifdef NC_FORMATX_NCZARR
#define NC_FORMATS(ID) ID < NC_FORMATX_UNDEFINED || ID > NC_FORMATX_NCZARR ?"unsupported":netcdf4async::format_names[ID]
#else
#define NC_FORMATS(ID) ID < NC_FORMATX_UNDEFINED || ID > NC_FORMATX_ZARR ?"unsupported":netcdf4async::format_names[ID]
#endif
#endif

#if NETCDF_VERSION_MINOR >= 8
#define NC_FORMATS(ID) ID < NC_FORMATX_UNDEFINED || ID > NC_FORMATX_NCZARR ?"unsupported":netcdf4async::format_names[ID]
#endif

namespace netcdf4async {

    extern const char *format_names[];
	extern const char *type_names[];

/**
 * printf like formatting for C++ with std::string
 * Original source: https://stackoverflow.com/a/26221725/11722
 */
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

	
inline int get_type(const std::string &type_str) {
	if (type_str == "byte" || type_str == "i1" || type_str == "b" || type_str == "B") {
		return NC_BYTE;
	} else if (type_str == "char") {
		return NC_CHAR;
	} else if (type_str == "short" || type_str == "i2" || type_str == "h" || type_str == "s") {
		return NC_SHORT;
	} else if (type_str == "int" || type_str == "i4" || type_str == "i" || type_str == "l") {
		return NC_INT;
	} else if (type_str == "float" || type_str == "f4" || type_str == "f") {
		return NC_FLOAT;
	} else if (type_str == "double" || type_str == "f8" || type_str == "d") {
		return NC_DOUBLE;
	} else if (type_str == "ubyte" || type_str == "u1") {
		return NC_UBYTE;
	} else if (type_str == "ushort" || type_str == "u2") {
		return NC_USHORT;
	} else if (type_str == "uint" || type_str == "u4") {
		return NC_UINT;
	} else if (type_str == "uint64" || type_str == "u8") {
		return NC_UINT64;
	} else if (type_str == "int64" || type_str == "i8") {
		return NC_INT64;
	} else if (type_str == "string" || type_str == "S1") {
		return NC_STRING;
	} else {
		return NC2_ERR;
	}
}

class File : public Napi::ObjectWrap<File> {
  public:
	static void Init(Napi::Env env);
	static Napi::Object Build(Napi::Env env,int ncid,std::string name,std::string mode,int type);
	explicit File(const Napi::CallbackInfo &info);
	~File();

  private:
	static Napi::FunctionReference constructor;

	bool open(const char *filename, const int &mode, const int &format);
	// Napi::Value New(const Napi::CallbackInfo &info);
	Napi::Value Close(const Napi::CallbackInfo &info);
	Napi::Value Sync(const Napi::CallbackInfo &info);
	Napi::Value DataMode(const Napi::CallbackInfo &info);
	Napi::Value GetName(const Napi::CallbackInfo &info);
	Napi::Value IsClosed(const Napi::CallbackInfo &info);
	Napi::Value GetFormat(const Napi::CallbackInfo &info);
	Napi::Value Inspect(const Napi::CallbackInfo &info);

	int id;
	std::string name;
	std::string mode;
	std::string format;
	Napi::Object group;
	bool closed;
};


} // namespace netcdf4async

#endif
