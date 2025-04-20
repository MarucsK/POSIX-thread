#pragma once

#include <iostream>
#include <memory>
#include <functional>
#include <utility>
#include <tuple>
#include <pthread.h>
#include <cstring>
#include <unistd.h>

namespace Marcus{

class thread{
public:
    class id;
    using native_handle_type = pthread_t;

    thread() noexcept : Id{} {}

    template<typename Fn, typename...Args>
    thread(Fn&& func, Args&&... args){
        using Tuple = std::tuple<std::decay_t<Fn>, std::decay_t<Args>...>;
        auto Decay_copied = std::make_unique<Tuple>(std::forward<Fn>(func), std::forward<Args>(args)...);
        auto Invoker_proc = start<Tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});

        if (int result = pthread_create(&Id, nullptr, Invoker_proc, Decay_copied.get()); result==0){
            (void)Decay_copied.release();
        }
        else{
            std::cerr << "Error creating thread: " << strerror(result) << std::endl;
            throw std::runtime_error("Error creating thread");
        }
    }

    template<class Tuple, std::size_t... Indices>
    static constexpr auto start(std::index_sequence<Indices...>) noexcept{
        return &Invoke<Tuple, Indices...>;
    }

    template<class Tuple, std::size_t... Indices>
    static void* Invoke(void* RawVals) noexcept{
        const std::unique_ptr<Tuple> FnVals(static_cast<Tuple *>(RawVals));
        Tuple &Tup = *FnVals.get();
        std::invoke(std::move(std::get<Indices>(Tup))...);
        return nullptr;
    }

    ~thread(){
        if(joinable()){
            std::terminate();
        }
    }

    thread(const thread &) = delete;

    thread &operator=(const thread &) = delete;

    thread(thread &&other) noexcept : Id(std::exchange(other.Id, {})) {}

    thread& operator=(thread&& t) noexcept{
        if(joinable()){
            std::terminate();
        }
        swap(t);
        return *this;
    }

    void swap(thread& t) noexcept{
        std::swap(Id, t.Id);
    }

    bool joinable() const noexcept{
        return !(Id == 0);
    }
    void join(){
        if(!joinable()){
            throw std::runtime_error("Thread is not joinable");
        }
        int result = pthread_join(Id, nullptr);
        if(result != 0){
            throw std::runtime_error("Error joining thread: " + std::string(strerror(result)));
        }
        Id = {}; // Reset thread id
    }

    void detach(){
        if(!joinable()){
            throw std::runtime_error("Thread is not joinable or already detached");
        }
        int result = pthread_detach(Id);
        if(result != 0){
            throw std::runtime_error("Error detaching thread: " + std::string(strerror(result)));
        }
        Id = {};
    }

    id get_id() const noexcept;

    native_handle_type native_handle() const{
        return Id;
    }

    pthread_t Id;
};

namespace this_thread{
    [[nodiscard]] thread::id get_id() noexcept; // 返回值应该被使用，否则警告
} // namespace this_thread

class thread::id{
public:
    id() noexcept = default;

private:
    explicit id(pthread_t other_id) noexcept : Id(other_id) {}

    pthread_t Id;

    friend thread::id thread::get_id() const noexcept;
    friend thread::id this_thread::get_id() noexcept;
    friend bool operator==(thread::id left, thread::id right) noexcept;

    template <class Ch, class Tr>
    friend std::basic_ostream<Ch, Tr> &
    operator<<(std::basic_ostream<Ch, Tr> &str, thread::id Id);
};

[[nodiscard]] inline thread::id thread::get_id() const noexcept{
    return thread::id(Id);
}

[[nodiscard]] inline thread::id this_thread::get_id() noexcept{
    return thread::id { pthread_self() };
}

template <class Ch, class Tr>
std::basic_ostream<Ch, Tr> &
operator<<(std::basic_ostream<Ch, Tr> &str, thread::id Id){
    str << Id.Id;
    return str;
}
} // namespace Marcus