#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

// Carries the old and new scene names
struct SceneChangedEvent {
    std::string oldSceneName;
    std::string newSceneName;
};

class EventDispatcher {

  public:
    using ListenerId = std::uint64_t;

    using Handler = std::function<void(const SceneChangedEvent&)>;

    // Saves a function to call when an event arrives
    // Returns an ID for the subscription
    ListenerId subscribe(Handler handler);

    // Removes that subscription so it stops receiving future events
    void unsubscribe(ListenerId listenerId);

    // Immediately calls the subscribed functions, giving each one the event
    void dispatch(const SceneChangedEvent& event);

  private:
    std::vector<std::pair<ListenerId, Handler>> subscriptions;

    ListenerId idCounter = 1;
};