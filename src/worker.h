#ifndef NETCDF4_WORKER_H
#define NETCDF4_WORKER_H

#include <napi.h>
#include <string>

template <class NetCDFType> class NetCDFPromiseWorker : public Napi::AsyncWorker {

  typedef std::function<NetCDFType()> NetCDFMainFunc;
  typedef std::function<Napi::Value(const NetCDFType)> NetCDFRValFunc;
  private:
    Napi::Promise::Deferred& deferred;
    Napi::Value result;
    const NetCDFMainFunc doit;
    const NetCDFRValFunc rval;

  public:
    explicit NetCDFPromiseWorker(Napi::Env env, Napi::Promise::Deferred &deferred, const NetCDFMainFunc doit, const NetCDFRValFunc rval);

    ~NetCDFPromiseWorker();
  protected:
    void OnError(const Napi::Error& e);
    void OnOK();
    void Execute();
  
};

template <class NetCDFType> NetCDFPromiseWorker<NetCDFType> ::NetCDFPromiseWorker(Napi::Env env,
     Napi::Promise::Deferred &promise,
     const NetCDFMainFunc mainFunc,
     const NetCDFRValFunc rvalFunc):
  Napi::AsyncWorker(env),
  deferred(promise),
  doit(mainFunc),
  rval(rvalFunc) {}

  template <class NetCDFType> void NetCDFPromiseWorker<NetCDFType> ::Execute() {
    result = rval(doit());
  }


template <class NetCDFType> void NetCDFPromiseWorker<NetCDFType> ::OnError(const Napi::Error& error) {
  Napi::HandleScope scope(Env());
  deferred.Reject(error.Value());
}


template <class NetCDFType> void NetCDFPromiseWorker<NetCDFType> ::OnOK() {
  Napi::HandleScope scope(Env());
  deferred.Resolve(this->result);
}

#endif