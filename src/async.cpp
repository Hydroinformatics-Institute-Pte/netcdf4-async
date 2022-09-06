#include <thread>
#include <functional>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"


namespace netcdf4async {

    template<class NetCDFType> NCAsyncWorker<NetCDFType>::NCAsyncWorker(
        Napi::Env &env,
        Napi::Promise::Deferred deferred, 
        const NCMainFunc doit, 
        const NCRValFunc rval
        ) : Napi::AsyncWorker(env), deferred(deferred), doit(doit),rval(rval),result(nullptr) {
            printf("Inited\n");
    }

    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::setResult(const NetCDFType result) {
        this->result=result;
    }

    template<class NetCDFType> const NetCDFType NCAsyncWorker<NetCDFType>::getResult() {
        return this->result;
    }

    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::Execute() {
        try {
            printf("Do it\n");
            this->setResult(doit(this));
            printf("Done\n");
        } catch (const char *err) { 
            printf("Catch\n");
            this->SetError(err); 
        }
    }

    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::OnOK() {
        printf("OnOk\n");
        auto env=Env();
        try {
            auto v=rval(env,this->getResult()); 
            deferred.Resolve(v); 
            printf("Done\n");
        } catch (const char *err) { 
            deferred.Reject(Napi::String::New(env,err)); 
        }

    }

    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::OnError(Napi::Error const &error) {
        printf("OnError\n");
        deferred.Reject(error.Value());
    }



}
