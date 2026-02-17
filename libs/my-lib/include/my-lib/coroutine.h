#ifndef __MY_LIB_COROUTINE_HEADER_H__
#define __MY_LIB_COROUTINE_HEADER_H__

#include <coroutine>

#include <my-lib/std.h>
#include <my-lib/memory.h>
#include <my-lib/memory-pool.h>
#include <my-lib/exception.h>


namespace Mylib
{

// ---------------------------------------------------

template <size_t buffer_size = 1024>
struct Coroutine {
	inline static Memory::PoolCore pool = Memory::PoolCore(buffer_size, 16, __STDCPP_DEFAULT_NEW_ALIGNMENT__);

	struct promise_type {
		// If the coroutine is not waiting for a timer event, this is nullptr.
		// Otherwise, it points to the object that owns of the awaiter.
		// We need this to be able to destroy the event when the coroutine finishes.
		void *awaiter_owner;
		void *awaiter_data;

		static void* operator new (const size_t size)
		{
			mylib_assert(size < buffer_size)
			//std::cout << "promise_type::operator new size " << size << " ptr " << ptr << std::endl;
			return pool.allocate();
		}

		static void operator delete (void *ptr, const size_t size)
		{
			//std::cout << "promise_type::operator delete size " << size << " ptr " << ptr << std::endl;
			pool.deallocate(ptr);
		}

		Coroutine get_return_object ()
		{
			return Coroutine {
				.handler = std::coroutine_handle<promise_type>::from_promise(*this)
			};
		}

		// Suspend the coroutine immediately after creation.
		std::suspend_always initial_suspend () const noexcept { return {}; }

		// https://stackoverflow.com/questions/75778999/should-promises-final-suspend-always-return-stdsuspend-always
		// Quoting:
		// It would be better to say that final_suspend should "always suspend",
		// rather than "always return std::suspend_always".
		// A coroutine is considered to be done if it is suspended at its
		// final-suspend point. So if a coroutine flows past this point without
		// suspending, then the coroutine is over, but coroutine_handle::done() will not be true.
		//
		// In summary, in order to be able to check if a coroutine is done, using
		// coroutine_handle::done(), the final_suspend should always suspend.
		std::suspend_always final_suspend () const noexcept { return {}; }

		// co_return returns nothing.
		void return_void () const noexcept {}

		void unhandled_exception () { std::terminate(); }
	};

	std::coroutine_handle<promise_type> handler;
};

template <typename Tcoroutine>
using PromiseType = typename Tcoroutine::promise_type;

template <typename Tcoroutine>
using CoroutineHandle = std::coroutine_handle<PromiseType<Tcoroutine>>;

// ---------------------------------------------------

template <typename Tcoroutine>
void initialize_coroutine (Tcoroutine coro)
{
	PromiseType<Tcoroutine>& promise = coro.handler.promise();
	promise.awaiter_owner = nullptr;
	promise.awaiter_data = nullptr;

	// We created the coroutine in a suspended state.
	// We need to resume it to start the execution.
	coro.handler.resume();
}

// ---------------------------------------------------

} // end namespace Mylib

#endif