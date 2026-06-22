#include <sstream>

#define DB_HANDLER_MACRO db_handler::Handler<ThreadCount, MutexCount, FileWorkerPath, Policy, IsMutexShared>

template <int ThreadCount, int MutexCount, db_handler::IsFilePath auto FileWorkerPath, Policies Policy, bool IsMutexShared>
std::future<std::optional<std::string>> DB_HANDLER_MACRO::Get(const std::string& key){
    auto promise_ptr = std::make_shared<std::promise<std::optional<std::string>>>();

    std::future<std::optional<std::string>> future_res = promise_ptr->get_future();
   
    thread_pool_.addTask([this, promise_ptr](const std::string& key) mutable{
        using LockType = std::conditional_t<IsMutexShared,
            std::shared_lock<std::shared_mutex>,
            std::lock_guard<std::mutex>
        >;
        size_t mtx_idx = GetMutexHash(key);
        try{
            std::string* res_ptr = nullptr;
            {
                LockType guard{fragments_mutexes[mtx_idx].mtx};
                res_ptr = cache_module_.Get(key);
                // found in cache
                promise_ptr->set_value(std::optional<std::string>(*res_ptr));
            }
            
            if (!res_ptr){
                // find in file worker
                std::optional<std::string> file_res = io_worker_.read(key);
                promise_ptr->set_value(file_res);    
                return;
            }
            
        }catch (...){
            promise_ptr->set_exception(std::current_exception());
        }
        
    }, key);


    return future_res;
}


template <int ThreadCount, int MutexCount, db_handler::IsFilePath auto FileWorkerPath, Policies Policy, bool IsMutexShared>
std::future<bool> DB_HANDLER_MACRO::Set(const std::string& key, const std::string& value){
    auto promise_ptr = std::make_shared<std::promise<bool>>();

    std::future<bool> future_res = promise_ptr->get_future();
   
    using LockType = std::conditional_t<IsMutexShared,
        std::unique_lock<std::shared_mutex>,
        std::lock_guard<std::mutex>
    >;
    thread_pool_.addTask([this, promise_ptr](const std::string& key, const std::string& value) mutable{
        size_t mtx_idx = GetMutexHash(key);
        try{
            {
                LockType guard{fragments_mutexes[mtx_idx].mtx};
                bool res = cache_module_.Put(key, value);
                promise_ptr->set_value(res);
            }

            io_worker_.write(key, value);
        }catch (...){
            promise_ptr->set_exception(std::current_exception());
        }
        
    }, key, value);

    return future_res;
}


template <int ThreadCount, int MutexCount, db_handler::IsFilePath auto FileWorkerPath, Policies Policy, bool IsMutexShared>
size_t DB_HANDLER_MACRO::GetMutexHash(const std::string& key){
    return hash_func_(key) % MutexCount;
}


