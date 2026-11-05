# EventDispatcher — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/events/event_dispatcher.h`](../../include/events/event_dispatcher.h) and [`src/events/event_dispatcher.cpp`](../../src/events/event_dispatcher.cpp).

**In one sentence:** A simple subscribe/notify hub. Code registers functions, and `dispatch` calls them all with a `SceneChangedEvent`.

**Why it exists:** [`SceneManager`](scene-manager.md) can announce "the scene changed" without knowing who cares. Listeners can be added or removed independently.

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `SceneChangedEvent` | Old and new scene names | [Struct](#struct-scenechangedevent) |
| `EventDispatcher` | The hub | [Class](#class-eventdispatcher) |
| `ListenerId`, `Handler` | Type aliases | [Types](#state-and-types) |
| `subscribe(Handler)` | Register a listener | [Function](#eventdispatcher-subscribe) |
| `unsubscribe(ListenerId)` | Remove a listener | [Function](#eventdispatcher-unsubscribe) |
| `dispatch(const SceneChangedEvent&)` | Notify everyone | [Function](#eventdispatcher-dispatch) |

## Struct: `SceneChangedEvent`

<a id="struct-scenechangedevent"></a>

| Field | Type | Meaning |
| --- | --- | --- |
| <a id="scenechangedevent-oldscenename"></a>`oldSceneName` | `std::string` | Previous scene's name; **empty** for the first scene |
| <a id="scenechangedevent-newscenename"></a>`newSceneName` | `std::string` | New active scene's name |

The names are **copies**, so they stay valid after the old scene is destroyed.

## State and types

<a id="state-and-types"></a>

| Name | Kind / type | Value | Meaning |
| --- | --- | --- | --- |
| <a id="eventdispatcher-listenerid"></a>`EventDispatcher::ListenerId` | alias `std::uint64_t` | — | Subscription ticket number |
| <a id="eventdispatcher-handler"></a>`EventDispatcher::Handler` | alias `std::function<void(const SceneChangedEvent&)>` | — | A listener function |

## Class: `EventDispatcher`

<a id="class-eventdispatcher"></a>

**How it's created and destroyed:** A plain object. In the game, [`engine`](engine.md#engine) creates it on the stack **before** the `SceneManager` (which borrows it), so it outlives the manager.

### Fields

| Field | Type / initial value | Meaning | Who changes it |
| --- | --- | --- | --- |
| <a id="eventdispatcher-subscriptions"></a>`subscriptions` | `std::vector<std::pair<ListenerId, Handler>>` | Listeners in registration order | `subscribe`, `unsubscribe` |
| <a id="eventdispatcher-idcounter"></a>`idCounter` | `ListenerId`, `1` | Next ID to give out (so IDs start at 1) | `subscribe` |

## Functions

<a id="eventdispatcher-subscribe"></a>
### `ListenerId EventDispatcher::subscribe(Handler handler)`

Takes the next ID, increments the counter, stores `{id, handler}` at the end, and returns the ID. IDs are never reused. **Related tests:** [SubscribeReturnsDifferentIds](../tests/test-event-dispatcher.md#eventdispatchertest-subscribereturnsdifferentids).

<a id="eventdispatcher-unsubscribe"></a>
### `void EventDispatcher::unsubscribe(ListenerId listenerId)`

Finds the entry with `std::find_if` and erases it. An unknown ID does nothing. **Related tests:** [UnsubscribedListenerIsNotCalled](../tests/test-event-dispatcher.md#eventdispatchertest-unsubscribedlistenerisnotcalled).

<a id="eventdispatcher-dispatch"></a>
### `void EventDispatcher::dispatch(const SceneChangedEvent& event)`

**What it does:** Calls every listener **synchronously, in registration order**.

**What happens:** it **copies** `subscriptions` first, then calls each copied handler.

**Why the copy:** a listener might subscribe or unsubscribe while being called. Because of the copy, those changes take effect only on the **next** dispatch, and the loop doesn't break.

**Errors:** if a listener throws, the exception leaves `dispatch` and the remaining listeners aren't called. Objects captured by listeners must stay alive during the dispatch.

**Used by:** [`SceneManager::setActiveScene`](scene-manager.md#scenemanager-setactivescene). **Related tests:** [DispatchCallsListenersInOrder](../tests/test-event-dispatcher.md#eventdispatchertest-dispatchcallslistenersinorder).

## Things worth knowing

- It handles only **one event type**. Adding more types later will need templates or overloads.
- Nothing in the game subscribes yet. Only tests do.

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
