#ifndef NCUTILS_H
#define NCUTILS_H
#include <netcdf_meta.h>


#if NODE_MAJOR_VERSION > 9
#define NC_TYPES(ID) ID < NC_BYTE || (ID > NC_UINT64 && ID != NC_STRING)?"unsupported":netcdf4async::type_names[ID]
#else
#define NC_TYPES(ID) ID < NC_BYTE || (ID > NC_UINT && ID != NC_STRING)?"unsupported":netcdf4async::type_names[ID]
#endif

#if NC_VERSION_MAJOR <4
#error NetCDF4 less then 4 not supported
#endif

#if NC_VERSION_MAJOR >4
#pragma message("Not tested with NetCDF4 greater than 4")
#endif

// Ubuntu 22.04 and Ubuntu 20.04 have a different name for NCZARR file type constant
// So....
#if NC_VERSION_MINOR < 7
#define NC_FORMATS(ID) ID < NC_FORMATX_UNDEFINED || ID > NC_FORMATX_DAP4 ?"unsupported":netcdf4async::format_names[ID]
#endif

#if NC_VERSION_MINOR >=7
#ifdef NC_FORMATX_NCZARR
#define NC_FORMATS(ID) ID < NC_FORMATX_UNDEFINED || ID > NC_FORMATX_NCZARR ?"unsupported":netcdf4async::format_names[ID]
#else
#define NC_FORMATS(ID) ID < NC_FORMATX_UNDEFINED || ID > NC_FORMATX_ZARR ?"unsupported":netcdf4async::format_names[ID]
#endif
#endif


namespace netcdf4async {

    extern const char *format_names[];
	extern const char *type_names[];

/**
 * @brief printf like formatting for C++ with std::string
 * printf like formatting for C++ with std::string
 * Original source: https://stackoverflow.com/a/26221725/11722
 * @tparam Args 
 * @param format 
 * @param args 
 * @return std::string 
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

/**
 * @brief Get the type object
 * 
 * @param type_str 
 * @return int 
 */
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

/**
 * @brief Get the type object
 * 
 * @param type_str 
 * @return int 
 */
inline std::string get_type_string(const int &type) {
	if (type == NC_BYTE) {
		return "byte";
	} else if (type == NC_CHAR) {
		return "char";
	} else if (type == NC_SHORT) {
		return "short";
	} else if (type == NC_INT) {
		return "int";
	} else if (type == NC_FLOAT) {
		return "float";
	} else if (type == NC_DOUBLE) {
		return "double";
	} else if (type == NC_UBYTE) {
		return "ubyte";
	} else if (type == NC_USHORT) {
		return "ushort";
	} else if (type == NC_UINT) {
		return "uint";
	} else if (type == NC_UINT64) {
		return "uint64";
	} else if (type == NC_INT64) {
		return "int64";
	} else if (type == NC_STRING) {
		return "string";
	} else {
		return "undefined type";
	}
}

}
#endif