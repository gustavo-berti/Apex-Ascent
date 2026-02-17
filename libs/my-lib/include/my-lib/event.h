#ifndef __MY_LIB_EVENT_HEADER_H__
#define __MY_LIB_EVENT_HEADER_H__

#include <iostream>
#include <list>
#include <functional>
#include <type_traits>
#include <memory>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/memory.h>
#include <my-lib/exception.h>

namespace Mylib
{
namespace Event
{

#ifdef MYLIB_EVENT_BASE_OPERATIONS
#error nooooooooooo
#endif

// ---------------------------------------------------

template <typename Tevent>
class Callback
{
public:
	virtual void operator() (Tevent& event) = 0;
	virtual ~Callback () = default;
};

// ---------------------------------------------------

/*
	Template parameter Tevent must be explicitly set.
	Function should be:
	void Tfunc (Tevent&);
*/

template <typename Tevent, typename Tfunc>
auto make_callback_function (Tfunc callback)
{
	class DerivedCallback : public Callback<Tevent>
	{
	private:
		Tfunc callback_function;

	public:
		DerivedCallback (Tfunc callback_function_)
			: callback_function(callback_function_)
		{
		}

		void operator() (Tevent& event) override
		{
			/*auto built_params = std::tuple_cat(
				std::forward_as_tuple(this->obj),
				std::forward_as_tuple(event)
			);
			std::apply(this->callback_function, built_params);*/
			std::invoke(this->callback_function, event);
		}
	};

	return DerivedCallback(callback);
}

// ---------------------------------------------------

/*
	Template parameter Tevent must be explicitly set.
	Object function should be:
	void Tobj::Tfunc (const Tevent&);
*/

template <typename Tevent, typename Tobj, typename Tfunc>
auto make_callback_object (Tobj& obj, Tfunc callback)
{
	class DerivedCallback : public Callback<Tevent>
	{
	private:
		Tobj& obj;
		Tfunc callback_function;

	public:
		DerivedCallback (Tobj& obj_, Tfunc callback_function_)
			: obj(obj_), callback_function(callback_function_)
		{
		}

		void operator() (Tevent& event) override
		{
			/*auto built_params = std::tuple_cat(
				std::forward_as_tuple(this->obj),
				std::forward_as_tuple(event)
			);
			std::apply(this->callback_function, built_params);*/
			std::invoke(this->callback_function, this->obj, event);
		}
	};

	return DerivedCallback(obj, callback);
}

// ---------------------------------------------------

template <typename Tevent, typename Tlambda_>
auto make_callback_lambda (Tlambda_&& callback)
{
	using Tlambda = typename remove_type_qualifiers<Tlambda_>::type;

	class DerivedCallback : public Callback<Tevent>
	{
	private:
		Tlambda callback_lambda;

	public:
		DerivedCallback (Tlambda_ callback_lambda_)
			: callback_lambda(callback_lambda_)
		{
		}

		void operator() (Tevent& event) override
		{
			/*auto built_params = std::tuple_cat(
				std::forward_as_tuple(this->obj),
				std::forward_as_tuple(event)
			);
			std::apply(this->callback_function, built_params);*/
			std::invoke(this->callback_lambda, event);
		}
	};

	return DerivedCallback(callback);
}

// ---------------------------------------------------

/*
	Template parameter Tevent must be explicitly set.
	Object function should be:
	void Tobj::Tfunc (const Tevent&, parameters...);

	Important, the first parameter is the Event Data (Tevent).
*/

template <typename Tevent, typename Tobj, typename Tfunc, typename Tfirst_param, typename... Args>
auto make_callback_object_with_params (Tobj& obj, Tfunc callback, const Tfirst_param& first_param, Args&&... args)
{
	//auto params = std::make_tuple(first_param, std::forward<Args>(args)...);
	auto params = std::make_tuple(first_param, args...);

	using Tparams = decltype(params);

	class DerivedCallback : public Callback<Tevent>
	{
	private:
		Tobj& obj;
		Tfunc callback_function;
		Tparams callback_params;
	
	public:
		DerivedCallback (Tobj& obj_, Tfunc callback_function_, const Tparams& callback_params_)
			: obj(obj_), callback_function(callback_function_), callback_params(callback_params_)
		{
		}

		void operator() (Tevent& event) override
		{
			auto built_params = std::tuple_cat(
				std::forward_as_tuple(this->obj), // can use std::tie as well
				std::forward_as_tuple(event),
				this->callback_params
			);
			
			std::apply(this->callback_function, built_params);
			//std::invoke(this->callback_function, *(this->obj));
		}
	};

	return DerivedCallback(obj, callback, params);
}

// ---------------------------------------------------

template <typename Tevent>
class Handler
{
public:
	using Type = Tevent;
	using EventCallback = Callback<Tevent>;

	struct Subscriber;

	struct Descriptor__ {
		Subscriber *subscriber = nullptr;
	};

	struct Descriptor {
		std::shared_ptr<Descriptor__> shared_ptr;

		bool is_valid () const noexcept
		{
			return (this->shared_ptr && this->shared_ptr->subscriber != nullptr);
		}
	};

	struct Subscriber {
		Descriptor descriptor;
		Memory::unique_ptr<EventCallback> callback; // used my unique_ptr to support polymorphic types
	};

private:
	using TallocSubscriber = Memory::AllocatorSTL<Subscriber>;
	//using TallocSubscriber = typename std::allocator_traits<Talloc>::template rebind_alloc<Subscriber>;

	Memory::Manager *memory_manager;
	std::list<Subscriber, TallocSubscriber> subscribers;

public:
	Handler ()
		: memory_manager(&Memory::default_manager),
		  subscribers(TallocSubscriber(*memory_manager))
	{
	}

	Handler (Memory::Manager& memory_manager_)
		: memory_manager(&memory_manager_),
		  subscribers(TallocSubscriber(*memory_manager))
	{
	}

	~Handler ()
	{
		for (auto& subscriber : this->subscribers) {
			subscriber.descriptor.shared_ptr->subscriber = nullptr;
		}
	}

	// delete copy constructor and assignment operator
	Handler (const Handler&) = delete;
	Handler& operator= (const Handler&) = delete;

	// provide default move constructor and assignment operator
	Handler (Handler&&) = default;
	Handler& operator= (Handler&&) = default;

	// We don't use const Tevent& because we allow the user to manipulate event data.
	// This is useful for the timer, allowing us to re-schedule events.
	void publish (Tevent& event)
	{
		for (auto& subscriber : this->subscribers) {
			auto& c = *(subscriber.callback);
			c(event);
		}
	}

	inline void publish (Tevent&& event)
	{
		this->publish(event);
	}

	/* When creating the event listener by r-value ref,
	   we allocate internal storage and copy the value to it.
	*/

	template <typename Tcallback>
	Descriptor subscribe (const Tcallback& callback)
		//requires std::is_rvalue_reference<decltype(callback)>::value
	{
		using TallocatorDescriptor = Memory::AllocatorSTL<Descriptor__>;
		TallocatorDescriptor descriptor_allocator(*this->memory_manager);

		auto unique_ptr = Memory::make_unique<Tcallback>(*this->memory_manager, callback);

		this->subscribers.push_back( Subscriber {
			.callback = std::move(unique_ptr),
			} );
		
		Subscriber& subscriber = this->subscribers.back();

		subscriber.descriptor = Descriptor {
			.shared_ptr = std::allocate_shared<Descriptor__>(descriptor_allocator, Descriptor__ {
				.subscriber = &subscriber
			})
		};

		return subscriber.descriptor;
	}

	void unsubscribe (Descriptor& descriptor)
	{
		bool found = false;

		this->subscribers.remove_if(
			[&descriptor, &found] (Subscriber& subscriber) -> bool {
				const bool local_found = (descriptor.shared_ptr->subscriber == &subscriber);

				if (local_found)
					found = true;
				
				return local_found;
			}
		);

		mylib_assert_exception(found, EventSubscriberNotFoundException)

		descriptor.shared_ptr->subscriber = nullptr;
		descriptor.shared_ptr.reset();
	}
};

// ---------------------------------------------------

} // end namespace Event
} // end namespace Mylib

#endif