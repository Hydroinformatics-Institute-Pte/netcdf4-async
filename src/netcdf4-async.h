#ifndef NETCDF4ASYNC_H
#define NETCDF4ASYNC_H

#include <napi.h>
#include <netcdf.h>
#include <string>
#include <memory>
#include <stdexcept>
#include "utils.h"


namespace netcdf4async {

  extern const char *format_names[];
	extern const char *type_names[];



class File : public Napi::ObjectWrap<File> {
  public:
	static void Init(Napi::Env env);
	static Napi::Object Build(Napi::Env env,int ncid,std::string name,std::string mode,int type);
	static Napi::Value Open (const Napi::CallbackInfo& info);
	
	explicit File(const Napi::CallbackInfo &info,int id,std::string name,std::string mode,int format);
	explicit File(const Napi::CallbackInfo &info);
	~File();

  private:
	static Napi::FunctionReference constructor;

	
	// Napi::Value New(const Napi::CallbackInfo &info);
	Napi::Value Close(const Napi::CallbackInfo &info);
	Napi::Value Sync(const Napi::CallbackInfo &info);
	Napi::Value DataMode(const Napi::CallbackInfo &info);
	Napi::Value GetName(const Napi::CallbackInfo &info);
	Napi::Value IsClosed(const Napi::CallbackInfo &info);
	Napi::Value GetFormat(const Napi::CallbackInfo &info);
	Napi::Value Inspect(const Napi::CallbackInfo &info);
	// Async calls

//	void close();
	

	int id;
	std::string name;
	std::string mode;
	int format;
	Napi::Object group;
	bool closed;
};


} // namespace netcdf4async

#endif
