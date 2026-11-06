#include "event_dispatcher.h"
#include <gtest/gtest.h>
#include <vector>

TEST(EventDispatcherTest, SubscribeReturnsDifferentIds) {
    EventDispatcher dispatcher;
    auto id1 = dispatcher.subscribe([](const SceneChangedEvent&) {});
    auto id2 = dispatcher.subscribe([](const SceneChangedEvent&) {});
    EXPECT_NE(id1, id2);
}

TEST(EventDispatcherTest, DispatchCallsListenersInOrder) {
    EventDispatcher dispatcher;

    std::vector<int> order;

    dispatcher.subscribe([&order](const SceneChangedEvent&) { order.push_back(1); });

    dispatcher.subscribe([&order](const SceneChangedEvent&) { order.push_back(2); });

    dispatcher.dispatch(SceneChangedEvent());

    EXPECT_EQ(order, std::vector<int>({1, 2}));
}

TEST(EventDispatcherTest, UnsubscribedListenerIsNotCalled) {

    EventDispatcher dispatcher;

    bool called = false;

    auto id = dispatcher.subscribe([&called](const SceneChangedEvent&) { called = true; });

    dispatcher.unsubscribe(id);

    dispatcher.dispatch(SceneChangedEvent());

    EXPECT_FALSE(called);
}