# test_event_dispatcher.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/events/test_event_dispatcher.cpp`](../../tests/events/test_event_dispatcher.cpp)

**In one sentence:** It checks that [`EventDispatcher`](../code/event-dispatcher.md) gives out unique IDs, calls listeners in order, and stops calling removed ones.

## Individual tests

<a id="eventdispatchertest-subscribereturnsdifferentids"></a>
### `EventDispatcherTest.SubscribeReturnsDifferentIds`

Two [`subscribe`](../code/event-dispatcher.md#eventdispatcher-subscribe) calls return different IDs.

<a id="eventdispatchertest-dispatchcallslistenersinorder"></a>
### `EventDispatcherTest.DispatchCallsListenersInOrder`

Listeners push 1, then 2. After [`dispatch`](../code/event-dispatcher.md#eventdispatcher-dispatch), the order is `{1, 2}`.

<a id="eventdispatchertest-unsubscribedlistenerisnotcalled"></a>
### `EventDispatcherTest.UnsubscribedListenerIsNotCalled`

Subscribe, [`unsubscribe`](../code/event-dispatcher.md#eventdispatcher-unsubscribe), dispatch. The listener isn't called.

## Coverage gaps

No tests for subscribing or unsubscribing **during** a dispatch (the snapshot rule), for a throwing listener, or for an unknown ID.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
