#include "async.h"
#include <napi.h>

namespace netcdf4async {
/**
     * @brief Construct a new Async Promise Worker:: Async Promise Worker object
     * 
     * @param env NodeJS environment
     * @param deferred Deferred promise object
     */
    AsyncPromiseWorker::AsyncPromiseWorker(
        Napi::Env &env,
        Napi::Promise::Deferred deferred
        ) : Napi::AsyncWorker(env), deferred(deferred) {
        }

    /**
     * @brief Construct a new Async Promise Worker:: Async Promise Worker object
     * 
     * @param env NodeJS environment
     */
    AsyncPromiseWorker::AsyncPromiseWorker(
        Napi::Env &env
        ) : Napi::AsyncWorker(env), deferred(Napi::Promise::Deferred::New(env)) {
        }

    /**
     * @brief Default OnError handle
     * Reject deferred with error message string
     * @param error Error
     */
    void AsyncPromiseWorker::OnError(Napi::Error const &error) {
        auto deferred=Deferred();
        deferred.Reject(error.Value());
    }

    /**
     * @brief Return deferred object
     * 
     * @return Napi::Promise::Deferred 
     */
    Napi::Promise::Deferred AsyncPromiseWorker::Deferred() {
        return deferred;
    }
}
