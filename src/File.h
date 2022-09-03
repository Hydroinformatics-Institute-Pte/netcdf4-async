#ifndef FILE_H
#define FILE_H

#include "NetCDFFile.h"
#include <napi.h>


class File : public Napi::ObjectWrap<File> {
  public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);
	explicit File(const Napi::CallbackInfo &info, NetCDFFile netcdf_file);
	~File();

  private:
	static Napi::FunctionReference constructor;

	Napi::Value Close(const Napi::CallbackInfo &info);
	Napi::Value Sync(const Napi::CallbackInfo &info);
	Napi::Value DataMode(const Napi::CallbackInfo &info);
	Napi::Value GetId(const Napi::CallbackInfo &info);
	Napi::Value GetName(const Napi::CallbackInfo &info);
	Napi::Value IsClosed(const Napi::CallbackInfo &info);
	Napi::Value GetFormat(const Napi::CallbackInfo &info);
	Napi::Value Inspect(const Napi::CallbackInfo &info);

    NetCDFFile *file;
};


#endif