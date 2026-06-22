#include <sstream>

#define DB_HANDLER_MACRO db_handler::Handler<ThreadCount, MutexCount, FileWorkerPath, Policy, Types...>

template <int ThreadCount, int MutexCount, db_handler::IsFilePath FileWorkerPath, Policies Policy, typename... Types>
std::future<typename DB_HANDLER_MACRO::Variant*> DB_HANDLER_MACRO::Get(const std::string& key){
    auto promise_ptr = std::make_shared<std::promise<Variant*>>();

    std::future<Variant*> future_res = promise_ptr->get_future();
   
    thread_pool_.addTask([this, promise_ptr](const std::string& key) mutable{
        try{
            Variant* res_ptr = cache_module_.Get(key);
            if (!res_ptr){
                // find in file worker
                // std::optional<
                // todo: uhhh, so we have serialisation issues and for now idk how to solve them
                return;
            }
            // found in cache
            promise_ptr->set_value(res_ptr);
        }catch (...){
            promise_ptr->set_exception(std::current_exception());
        }
        
    }, key);


    return future_res;
}


template <int ThreadCount, int MutexCount, db_handler::IsFilePath FileWorkerPath, Policies Policy, typename... Types>
std::future<bool> DB_HANDLER_MACRO::Set(const std::string& key, DB_HANDLER_MACRO::Variant value){
    auto promise_ptr = std::make_shared<std::promise<bool>>();

    std::future<bool> future_res = promise_ptr->get_future();
   
    thread_pool_.addTask([this, promise_ptr](const std::string& key, Variant& value) mutable{
        try{
            bool res = cache_module_.Put(key, value);
            promise_ptr->set_value(res);


            // TODO
            // io_worker_.write()           // cant write to file yet cuz we have serialisation issues
        }catch (...){
            promise_ptr->set_exception(std::current_exception());
        }
        
    }, key, value);

    return future_res;
}
