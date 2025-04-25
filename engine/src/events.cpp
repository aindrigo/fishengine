#include "fish/events.hpp"
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include "cpptrace/from_current.hpp"
namespace fish
{
    void EventDispatcher::observe(const std::string& name, const std::function<EventObserver>& observer)
    {
        if (!this->observers.contains(name))
            this->observers[name] = {};

        this->observers[name].emplace_back(observer);
    }

    void EventDispatcher::observeImmutable(const std::string& name, const std::function<ImmutableEventObserver>& observer)
    {
        if (!this->immutableObservers.contains(name))
            this->immutableObservers[name] = {};

        this->immutableObservers[name].push_back(observer);
    }

    void EventDispatcher::observeAll(const std::function<MultiEventObserver>& observer)
    {
        this->multiObservers.emplace_back(observer);
    }

    EventData EventDispatcher::dispatch(const std::string& name, const EventData& data)
    {
        EventData result = data;
        if (this->observers.contains(name)) {
            for (auto const& observer : this->observers[name]) {
                CPPTRACE_TRY {
                    result = observer(result);
                } CPPTRACE_CATCH (std::exception& e) {
                    std::cout << std::format("Caught an exception dispatching event {}, continuing anyway: {}", name, e.what()) << std::endl;
#ifndef NDEBUG
                    cpptrace::from_current_exception().print();
#endif
                }
            }
        }

        for (auto const& observer : this->multiObservers) {
            CPPTRACE_TRY {
                result = observer(name, result);
            } CPPTRACE_CATCH (std::exception& e) {
                std::cout << std::format("Caught an exception dispatching event {}, continuing anyway: {}", name, e.what()) << std::endl;
#ifndef NDEBUG
                cpptrace::from_current_exception().print();
#endif
            }
        }

        if (this->immutableObservers.contains(name)) {
            for (auto const& observer : this->immutableObservers[name]) {
                CPPTRACE_TRY {
                    observer(result);
                } CPPTRACE_CATCH (std::exception& e) {
                    std::cout << std::format("Caught an exception dispatching event {}, continuing anyway: {}", name, e.what()) << std::endl;
#ifndef NDEBUG
                    cpptrace::from_current_exception().print();
#endif
                }
            }
        }

        return result;
    }
}