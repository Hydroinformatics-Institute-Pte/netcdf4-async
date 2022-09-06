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

    /**
     * @brief Asynchronus promise worker
     * Provide some basic functionality for promises
     */
    class AsyncPromiseWorker : public Napi::AsyncWorker {
        public:
            /// @brief Constructor
            /// @param env NodeJS environment
            /// @param deferred Deferred promise
            explicit AsyncPromiseWorker(Napi::Env &env, Napi::Promise::Deferred deferred);
            /// @brief Create worker with empty promise
            /// @param env NodeJS environment
            explicit AsyncPromiseWorker(Napi::Env &env);

            
            /// @brief Destroy the Async Promise Worker object
            ~AsyncPromiseWorker() override { };
            /// @brief Default OnError handler. Rejects promise
            /// @param error Error
            void OnError(Napi::Error const &error) override;
            /// @brief Getter for deferred
            /// @return Deferred promise object
            Napi::Promise::Deferred Deferred();
        private:
            Napi::Promise::Deferred deferred;

    };

    // /**
    //  * @brief Construct a new Async Promise Worker:: Async Promise Worker object
    //  * 
    //  * @param env NodeJS environment
    //  * @param deferred Deferred promise object
    //  */
    // AsyncPromiseWorker::AsyncPromiseWorker(
    //     Napi::Env &env,
    //     Napi::Promise::Deferred deferred
    //     ) : Napi::AsyncWorker(env), deferred(deferred) {
    //     }

    // /**
    //  * @brief Construct a new Async Promise Worker:: Async Promise Worker object
    //  * 
    //  * @param env NodeJS environment
    //  */
    // AsyncPromiseWorker::AsyncPromiseWorker(
    //     Napi::Env &env
    //     ) : Napi::AsyncWorker(env), deferred(Napi::Promise::Deferred::New(env)) {
    //     }

    // /**
    //  * @brief Default OnError handle
    //  * Reject deferred with error message string
    //  * @param error Error
    //  */
    // void AsyncPromiseWorker::OnError(Napi::Error const &error) {
    //     auto deferred=Deferred();
    //     deferred.Reject(error.Value());
    // }

    // /**
    //  * @brief Return deferred object
    //  * 
    //  * @return Napi::Promise::Deferred 
    //  */
    // Napi::Promise::Deferred AsyncPromiseWorker::Deferred() {
    //     return deferred;
    // }

    /**
     * @brief  Common class for handling async operations 
     * This is the common class for handling async operations 
     * It takes the lambdas as input
     * 
     *  JS-visible object creation is possible only in the main thread while
     *  ths JS world is not running
     * @tparam NetCDFType type of the object that will be carried from
     * the aux thread to the main thread
     */
    template <class NetCDFType> class NCAsyncWorker : public AsyncPromiseWorker {
        
        public:

            
            typedef std::function<NetCDFType(const NCAsyncWorker* worker)> NCMainFunc;
            typedef std::function<Napi::Value(Napi::Env env,NetCDFType result)> NCRValFunc;

            /// @brief Constructor
            /// @param env NodeJs environment
            /// @param deferred Deferred object
            /// @param doit is the user function lambda that produces the <NetCDFType> object 
            /// @param rval is the user function that will create the returned value
            explicit NCAsyncWorker(Napi::Env &env, Napi::Promise::Deferred &deferred, const NCMainFunc doit, const NCRValFunc rval);
            /// @brief Constructor
            /// @param env NodeJs environment
            /// @param doit is the user function lambda that produces the <NetCDFType> object 
            /// @param rval is the user function that will create the returned value
            explicit NCAsyncWorker(Napi::Env &env, const NCMainFunc doit, const NCRValFunc rval);
            /// @brief Destroy the NCAsyncWorker object
            ~NCAsyncWorker() override {};

            /// @brief Execute doit function
            void Execute() override;
            /// @brief Handler if doit finished successfully
            void OnOK() override;
            /// @brief Handle result value
            /// @param result 
            void setResult(NetCDFType result);
            /// @brief Get result value
            /// @return result
            const NetCDFType getResult();

        private:
            const NCMainFunc doit;
            const NCRValFunc rval;
            NetCDFType result;

    };

    /**
     * @brief Construct a new NCAsyncWorker<NetCDFType>::NCAsyncWorker object
     * 
     * @tparam NetCDFType type of the object that will be carried from
     * the aux thread to the main thread
     * @param env NodeJs environment
     * @param deferred Deferred object
     * @param doit is the user function lambda that produces the <NetCDFType> object 
     * @param rval is the user function that will create the returned value
     */
    template<class NetCDFType> NCAsyncWorker<NetCDFType>::NCAsyncWorker(
        Napi::Env &env,
        Napi::Promise::Deferred &deferred, 
        const NCMainFunc doit, 
        const NCRValFunc rval
        ) : AsyncPromiseWorker(env,deferred),  doit(doit),rval(rval) {
    }

    /**
     * @brief Construct a new NCAsyncWorker<NetCDFType>::NCAsyncWorker object
     * 
     * @tparam NetCDFType 
     * @tparam NetCDFType type of the object that will be carried from
     * the aux thread to the main thread
     * @param env NodeJs environment
     * @param deferred Deferred object
     * @param doit is the user function lambda that produces the <NetCDFType> object 
     * @param rval is the user function that will create the returned value
     */
    template<class NetCDFType> NCAsyncWorker<NetCDFType>::NCAsyncWorker(
        Napi::Env &env,
        const NCMainFunc doit, 
        const NCRValFunc rval
        ) : AsyncPromiseWorker(env),  doit(doit),rval(rval) {
    }

    /**
     * @brief Set result of async operation
     * 
     * @tparam NetCDFType 
     * @param result 
     */
    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::setResult(const NetCDFType result) {
        this->result=result;
    }

    /**
     * @brief Get stored result of async operation
     * 
     * @tparam NetCDFType 
     * @return const NetCDFType 
     */
    template<class NetCDFType> const NetCDFType NCAsyncWorker<NetCDFType>::getResult() {
        return this->result;
    }

    /**
     * @brief Execute async operation
     * Executes doit function and save result of execution
     * @tparam NetCDFType 
     */
    template<class NetCDFType> void NCAsyncWorker<NetCDFType>::Execute() {
        try {
            NetCDFType result=doit(this);
            setResult(result);
        } catch (const char *err) { 
            SetError(err); 
        }
    }

    /**
     * @brief Handle successfully execute of async operation
     * Get the NodeJS value from rval function and resolve promise
     * @tparam NetCDFType 
     */
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