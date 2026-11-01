#include "event_dispatcher.h"
#include <algorithm>

EventDispatcher::ListenerId EventDispatcher::subscribe(Handler handler) {
    auto subscriptionId = idCounter;
    idCounter++;
    subscriptions.push_back({subscriptionId, std::move(handler)});
    return subscriptionId;
}

void EventDispatcher::unsubscribe(ListenerId listenerId) {
    auto iterator =
        std::find_if(subscriptions.begin(), subscriptions.end(),
                     [listenerId](const auto& entry) { return entry.first == listenerId; });

    if (iterator != subscriptions.end()) {
        subscriptions.erase(iterator);
    }
}

/* Listeners run synchronously in registration order
Subscription changes affect subsequent dispatches; the current snapshot stays unchanged
Objects referenced by callbacks must remain alive through the current dispatch
If a callback throws, the exception propagates and the remaining callbacks aren’t called */
void EventDispatcher::dispatch(const SceneChangedEvent& event) {
    auto copySubscriptions = subscriptions;
    for (const auto& subscription : copySubscriptions) {
        subscription.second(event);
    }
}