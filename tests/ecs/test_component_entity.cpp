#include "component.h"
#include "entity.h"
#include <gtest/gtest.h>
#include <stdexcept>

class FakeComponent : public Component {};
class FakeComponent2 : public Component {};

// ---------- Entity / Component ----------
TEST(EntityTest, AddComponentReturnsTheComponent) {
    Entity entity(1);
    FakeComponent& component = entity.addComponent<FakeComponent>();
    EXPECT_EQ(&component, entity.getComponent<FakeComponent>());
}

TEST(EntityTest, AddingDuplicateComponentThrows) {
    Entity entity(1);
    entity.addComponent<FakeComponent>();
    EXPECT_THROW(entity.addComponent<FakeComponent>(), std::logic_error);
}

TEST(EntityTest, GetMissingComponentReturnsNullptr) {
    Entity entity(1);
    EXPECT_EQ(nullptr, entity.getComponent<FakeComponent>());
}

class UpdateTracker : public Component {
  public:
    bool updated = false;
    void update(float dt) override {
        updated = true;
    }
};

TEST(EntityTest, UpdateReachesComponents) {

    Entity entity(1);
    UpdateTracker& tracker = entity.addComponent<UpdateTracker>();
    entity.update(0.1f);
    EXPECT_TRUE(tracker.updated);
}