#ifndef NETCDF4JSTYPES_H
#define NETCDF4JSTYPES_H

#include <napi.h>
#include <string>

class NetCDFPromiseWorker : public Napi::AsyncWorker {
    private:
  Napi::Env env;

    public:
  explicit NetCDFPromiseWorker(
    Napi::Env env);

  ~NetCDFPromiseWorker();

  void HandleOKCallback();
  void HandleErrorCallback();
  inline Local<Value> Promise() {
    
  };
};

#endif