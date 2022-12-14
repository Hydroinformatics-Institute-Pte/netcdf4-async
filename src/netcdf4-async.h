#ifndef NETCDF4ASYNC_H
#define NETCDF4ASYNC_H

#include <napi.h>
#include <netcdf.h>
#include <string>
#include <memory>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include "utils.h"


namespace netcdf4async {

  /// @brief List of variable/attribute types
  extern const char *format_names[];
  /// @brief List of file format names
  extern const char *type_names[];

/// @brief NetCDF file implementation
class File : public Napi::ObjectWrap<File> {
  public:
	/// @brief lock

	/// @brief Initialize class as NodeJS object
	/// @param env NodeJS env
	static void Init(Napi::Env env);
	/// @brief Async open of NetCDF file
	/// @param info 
	/// @return Deferred promise
	static Napi::Value Open (const Napi::CallbackInfo& info);

    /// @brief Construct a new File object
    /// Construct NodeJS wrapper for netcdf file.
    /// @param info NodeJS params
    /// @param id file id
    /// @param name file path
    /// @param mode file mode
    /// @param format file format
	explicit File(const Napi::CallbackInfo &info,int id,std::string name,std::string mode,int format);
	/// @brief Async open netCDF file
	/// @param info NodeJS parameters
	explicit File(const Napi::CallbackInfo &info);
	~File();

  private:

	static Napi::FunctionReference constructor;
	static Napi::Object Build(Napi::Env env,int ncid,std::string name,std::string mode,int type);

	/// @brief NetCDF file id
	int id;
	/// @brief file path
	std::string name;
	/// @brief file mode
	std::string mode;
	/// @brief file format
	int format;
	/// @brief Root variable groop
	Napi::Object group;
	/// @brief Is file close
	bool closed;

	/// @brief Created and added default group to File
	/// @param env 
	/// @param  
	void createDefaultGroup(Napi::Env env,std::string name);


	// Napi::Value New(const Napi::CallbackInfo &info);

	/// @brief Perform asyc close
	/// @param info 
	/// @return Deferred promise
	Napi::Value Close(const Napi::CallbackInfo &info);
	/// @brief Perform async synchronization
	/// @param info 
	/// @return Deferred promise
	Napi::Value Sync(const Napi::CallbackInfo &info);
	/// @brief Switch NetCDF file from definition mode
	/// @param info 
	/// @return Deferred promise
	Napi::Value DataMode(const Napi::CallbackInfo &info);
	/// @brief Return file path
	/// @param info 
	/// @return NodeJS String
	Napi::Value GetName(const Napi::CallbackInfo &info);
	/// @brief Is file closed
	/// @param info 
	/// @return NodeJS boolean
	Napi::Value IsClosed(const Napi::CallbackInfo &info);
	/// @brief File format
	/// @see *format_names
	/// @param info 
	/// @return NodeJS file format string
	Napi::Value GetFormat(const Napi::CallbackInfo &info);
	/// @brief Object.toString impl
	/// @param info 
	/// @return 
	Napi::Value Inspect(const Napi::CallbackInfo &info);
};


class Group : public Napi::ObjectWrap<Group> {
  public:
	Group(const Napi::CallbackInfo &info);
	static void Init(Napi::Env env);
	static Napi::Object Build(Napi::Env env, int id, std::string name);
	bool get_name(char *name) const;
	void set_name(std::string groupname);

  private:
	static Napi::FunctionReference constructor;

	Napi::Value GetId(const Napi::CallbackInfo &info);
	Napi::Value GetName(const Napi::CallbackInfo &info);
	Napi::Value SetName(const Napi::CallbackInfo &info);
	Napi::Value GetPath(const Napi::CallbackInfo &info);

	Napi::Value GetSubgroups(const Napi::CallbackInfo &info);
	Napi::Value AddSubgroup(const Napi::CallbackInfo &info);
	Napi::Value GetSubgroup(const Napi::CallbackInfo &info);

	Napi::Value GetVariables(const Napi::CallbackInfo &info);
	Napi::Value GetVariable(const Napi::CallbackInfo &info);
	Napi::Value AddVariable(const Napi::CallbackInfo &info);

	Napi::Value GetDimensions(const Napi::CallbackInfo &info);
	Napi::Value AddDimension(const Napi::CallbackInfo &info);

	Napi::Value GetAttributes(const Napi::CallbackInfo &info);
	Napi::Value AddAttribute(const Napi::CallbackInfo &info);

	Napi::Value Inspect(const Napi::CallbackInfo &info);

	int id;
	std::string name;
};

class Variable : public Napi::ObjectWrap<Variable> {
  public:
	static void Init(Napi::Env env);
	Variable(const Napi::CallbackInfo &info);
	static Napi::Object Build(Napi::Env env, int id, int parent_id, std::string name, nc_type type, int ndims);
	void set_name(std::string new_name);
	
  private:

	Napi::Value GetName(const Napi::CallbackInfo &info);
	Napi::Value SetName(const Napi::CallbackInfo &info);
	Napi::Value GetNameSync(const Napi::CallbackInfo &info);
	Napi::Value GetTypeSync(const Napi::CallbackInfo &info);

	Napi::Value GetDimensions(const Napi::CallbackInfo &info);

	Napi::Value GetFill(const Napi::CallbackInfo &info);
	Napi::Value GetFillMode(const Napi::CallbackInfo &info);
	Napi::Value SetFill(const Napi::CallbackInfo &info);
 	Napi::Value _getFillInfo(Napi::Env env,bool return_mode);

	Napi::Value GetChunked(const Napi::CallbackInfo &info);
	Napi::Value SetChunked(const Napi::CallbackInfo &info);

	Napi::Value GetDeflateInfo(const Napi::CallbackInfo &info);
	Napi::Value SetDeflateInfo(const Napi::CallbackInfo &info);

	Napi::Value GetEndianness(const Napi::CallbackInfo &info);
	Napi::Value SetEndianness(const Napi::CallbackInfo &info);
	Napi::Value GetChecksumMode(const Napi::CallbackInfo &info);
	Napi::Value SetChecksumMode(const Napi::CallbackInfo &info);

	Napi::Value GetAttributes(const Napi::CallbackInfo &info);
	Napi::Value AddAttribute(const Napi::CallbackInfo &info);
	Napi::Value SetAttribute(const Napi::CallbackInfo &info);
	Napi::Value RenameAttribute(const Napi::CallbackInfo &info);
	Napi::Value DeleteAttribute(const Napi::CallbackInfo &info);


	Napi::Value Read(const Napi::CallbackInfo &info);
	Napi::Value ReadSlice(const Napi::CallbackInfo &info);
	Napi::Value ReadStridedSlice(const Napi::CallbackInfo &info);
	Napi::Value Write(const Napi::CallbackInfo &info);
	Napi::Value WriteSlice(const Napi::CallbackInfo &info);
	Napi::Value WriteStridedSlice(const Napi::CallbackInfo &info);
	Napi::Value Inspect(const Napi::CallbackInfo &info);
	static Napi::FunctionReference constructor;

	static const unsigned char type_sizes[];
	int id;
	int parent_id;
	nc_type type;
	int ndims;
	std::string name;
};

} // namespace netcdf4async

#endif
