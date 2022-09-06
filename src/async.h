#ifndef NCASYNC_H
#define NCASYNC_H

#include <napi.h>
#include <netcdf.h>
#include <string>
#include <memory>
#include <stdexcept>

#define NC_CALL(FN)                                                                                \
	do {                                                                                           \
		int retval = FN;                                                                           \
		if (retval != NC_NOERR) {                                                                  \
			throw std::runtime_error(string_format("NetCDF4: %s",nc_strerror(retval)));            \
		}                                                                                          \
	} while (false);

#define NC_COND_CALL(FN,CONDITIONS)                                                                                \
	do {                                                                                           \
		int retval = FN;                                                                           \
		if (retval != NC_NOERR && CONDITIONS) {                                                                  \
			throw std::runtime_error(string_format("NetCDF4: %s",nc_strerror(retval)));            \
		}                                                                                          \
	} while (false);

#define NC_VOID_CALL(FN)                                                                                \
	do {                                                                                           \
		FN;                                                                           \
	} while (false);

namespace netcdf4async {

    class AsyncPromiseWorker : public Napi::AsyncWorker {
        public:
            explicit AsyncPromiseWorker(Napi::Env &env, Napi::Promise::Deferred deferred);
            ~AsyncPromiseWorker() override { };
            void OnError(Napi::Error const &error) override;
            Napi::Promise::Deferred Deferred();
        private:
            Napi::Promise::Deferred deferred;

    };

    AsyncPromiseWorker::AsyncPromiseWorker(
        Napi::Env &env,
        Napi::Promise::Deferred deferred
        ) : Napi::AsyncWorker(env), deferred(deferred) {
        }

    void AsyncPromiseWorker::OnError(Napi::Error const &error) {
        auto deferred=Deferred();
        deferred.Reject(error.Value());
    }

    Napi::Promise::Deferred AsyncPromiseWorker::Deferred() {
        return deferred;
    }


    template <class NetCDFType> class NCAsyncWorker : public AsyncPromiseWorker {
        
        public:

            typedef std::function<NetCDFType(const NCAsyncWorker* worker)> NCMainFunc;
            typedef std::function<Napi::Value(Napi::Env env,NetCDFType result)> NCRValFunc;

            explicit NCAsyncWorker(Napi::Env &env, Napi::Promise::Deferred &deferred, const NCMainFunc doit, const NCRValFunc rval);
            ~NCAsyncWorker() override {};

            void Execute() override;
            void OnOK() override;
            void setResult(NetCDFType result);
            const NetCDFType getResult();

        private:
            const NCMainFunc doit;
            const NCRValFunc rval;
            NetCDFType result;

    };

    template<class NetCDFType> NCAsyncWorker<NetCDFType>::NCAsyncWorker(
        Napi::Env &env,
        Napi::Promise::Deferred &deferred, 
        const NCMainFunc doit, 
        const NCRValFunc rval
        ) : AsyncPromiseWorker(env,deferred),  doit(doit),rval(rval) {
    }

    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::setResult(const NetCDFType result) {
        this->result=result;
    }

    template<class NetCDFType> const NetCDFType NCAsyncWorker<NetCDFType>::getResult() {
        return this->result;
    }

    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::Execute() {
        try {
            NetCDFType result=doit(this);
            setResult(result);
        } catch (const char *err) { 
            SetError(err); 
        }
    }

    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::OnOK() {
        auto env=Env();
        Napi::Promise::Deferred deferred=Deferred();
        try {
            auto v=rval(env,getResult()); 
            deferred.Resolve(v); 
        } catch (const char *err) { 
            deferred.Reject(Napi::String::New(env,err));
        }

    }

}

#endif