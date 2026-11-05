# Entity — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/ecs/entity.h`](../../include/ecs/entity.h) and [`src/ecs/entity.cpp`](../../src/ecs/entity.cpp).

**In one sentence:** An `Entity` is a game object with an ID that owns a list of components and forwards `update`/`render` to each of them.

**Why it exists:** It is the container that groups related components (position, image, animation) into one "thing" in the scene.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | Owning components, finding them by type, forwarding update/render |
| Used by | [Scene](scene.md) (owns entities), [engine](engine.md) (adds demo components), components (find siblings) |
| Depends on | [Component](component.md), standard library |

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `Entity` | Game object | [Class](#class-entity) |
| `Entity(uint64_t)` | Create with an ID | [Function](#entity-entity) |
| `getId()` | Read the ID | [Function](#entity-getid) |
| `update(float)` | Update all components | [Function](#entity-update) |
| `render(Renderer&)` | Render all components | [Function](#entity-render) |
| `getComponent<T>()` | Find a component by exact type | [Function](#entity-getcomponent) |
| `addComponent<T>(args…)` | Create and attach a component | [Function](#entity-addcomponent) |

## Class: `Entity`

<a id="class-entity"></a>

**Purpose:** One object in a scene.

**How it's created and destroyed:** Usually by [`Scene::createEntity`](scene.md#scene-createentity), which gives it the next ID and owns it via `std::unique_ptr`. Tests also make entities on the stack. The default destructor destroys the `components` vector, which deletes every component.

**Copy and move:** All four (copy/move constructor and assignment) are **deleted**. Components hold a raw `Entity*` back-pointer, so moving or copying an entity would leave those pointers wrong. Deleting the operations makes that impossible.

**Important rules:** at most **one component of each exact type**. Component order = the order they were added.

### Fields

| Field | Type / initial value | Meaning | Who changes it / ownership |
| --- | --- | --- | --- |
| <a id="entity-id"></a>`id` | `std::uint64_t` | Identifier, unique **within its scene** | Set by constructor; read-only after |
| <a id="entity-components"></a>`components` | `std::vector<std::unique_ptr<Component>>` | Attached components in insertion order | Appended by [`addComponent`](#entity-addcomponent). **Owning.** |

## Functions

<a id="entity-entity"></a>
### `explicit Entity::Entity(std::uint64_t id)`

Stores `id` using a member initializer list. `explicit` prevents accidental conversion from a number. **No direct project-function calls.**

<a id="entity-getid"></a>
### `std::uint64_t Entity::getId() const`

Returns the ID. **Related tests:** [SceneTest.CreatesEntitiesWithUniqueIncreasingIds](../tests/test-scene.md#scenetest-createsentitieswithuniqueincreasingids).

<a id="entity-update"></a>
### `void Entity::update(float deltaTime)`

Calls [`Component::update`](component.md#component-update) on each component, in insertion order. Virtual dispatch reaches overrides such as [`AnimationComponent::update`](animation-component.md#animationcomponent-update). **Called by:** [`Scene::update`](scene.md#scene-update). **Related tests:** [EntityTest.UpdateReachesComponents](../tests/test-component-entity.md#entitytest-updatereachescomponents).

<a id="entity-render"></a>
### `void Entity::render(Renderer& renderer)`

Calls [`Component::render`](component.md#component-render) on each component, in insertion order. Reaches [`Sprite::render`](sprite.md#sprite-render). **Called by:** [`Scene::render`](scene.md#scene-render).

<a id="entity-getcomponent"></a>
### `template <typename T> T* Entity::getComponent()`

**What it does:** Finds the component whose **exact** runtime type is `T`.

**Returns:** a pointer to it, or `nullptr` if there's none. The pointer is **non-owning** and valid while the entity lives.

**What happens:**

1. `static_assert` that `T` derives from `Component` (a compile-time error otherwise).
2. Loops through `components` and compares `typeid(*component) == typeid(T)`.
3. Returns `static_cast<T*>` on the first match, or `nullptr`.

**Edge cases:**

- **Exact type only:** a subclass of `Sprite` would **not** be found by `getComponent<Sprite>()`.
- The compiler warns `expression with side effects will be evaluated despite being used as an operand to 'typeid'` on this line (the warning seen on every build). It's harmless here, because dereferencing has no real side effect. Writing `const Component& c = *component; typeid(c)` would silence it.
- It's linear search, which is fine for a handful of components.

**Used by:** [`addComponent`](#entity-addcomponent), [`Sprite::render`](sprite.md#sprite-render), [`AnimationComponent::update`](animation-component.md#animationcomponent-update), tests.

<a id="entity-addcomponent"></a>
### `template <typename T, typename... Args> T& Entity::addComponent(Args&&... args)`

**What it does:** Builds a new `T` with `args`, attaches it, and returns a reference to it.

| Name | Type | Meaning |
| --- | --- | --- |
| `T` | template type | The component class; must derive from `Component` |
| `args…` | forwarded | Passed to `T`'s constructor (e.g. a `textureHandle` for `Sprite`) |

**Returns:** `T&`, a **borrowed** reference. The entity owns the component.

**What happens, in order**

1. `static_assert` that `T` is a `Component`.
2. If [`getComponent<T>()`](#entity-getcomponent) already finds one, it throws `std::logic_error("This component already exists")`.
3. `std::make_unique<T>(std::forward<Args>(args)...)` creates it (the constructor may throw, e.g. `AnimationComponent`).
4. `setOwner(*this)` sets the back-pointer (allowed because `Entity` is a friend of `Component`).
5. Saves a reference, moves the `unique_ptr` into `components`, and returns the reference.

**Calls other project functions**

| Calls | Why | Where |
| --- | --- | --- |
| `getComponent<T>` | Reject duplicates | [getComponent](#entity-getcomponent) |
| `T::T(args…)` | Construct the component | e.g. [Sprite](sprite.md#sprite-sprite), [AnimationComponent](animation-component.md#animationcomponent-animationcomponent) |
| `Component::setOwner` | Link component to entity | [setOwner](component.md#component-setowner) |

**Errors:** `std::logic_error` on a duplicate; any exception from `T`'s constructor (nothing is attached in that case). **Note:** `entity.h` uses `std::logic_error` without including `<stdexcept>`. It compiles today only because another header includes it.

**Related tests:** [EntityTest](../tests/test-component-entity.md), [SpriteTest.CannotAttachDuplicateSprite](../tests/test-transform-sprite.md#spritetest-cannotattachduplicatesprite).

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
