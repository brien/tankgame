#include <gtest/gtest.h>
#include "GameWorld.h"
#include "LevelHandler.h"
#include "events/Events.h"

namespace {
struct Ping : EventBase<Ping> {};
struct Pong : EventBase<Pong> {};

CreateFXEvent Effect() {
    return CreateFXEvent(static_cast<int>(FxType::TYPE_STAR), 30, 2, 30,
                         0, 0, 0, 1, 1, 1, 1);
}

// Test observers also remove their own subscriptions, including on assertion failure.
struct Observer {
    ~Observer() { Events::Unsubscribe(this); }
    int calls = 0;
};
}

TEST(EventBusTest, UnsubscribePreservesOtherOwnersAndQueuedEvents) {
    EventBus bus;
    int first = 0, second = 0, unowned = 0;
    bus.Subscribe<Ping>([&](const Ping&) { ++first; }, &first);
    bus.Subscribe<Pong>([&](const Pong&) { ++first; }, &first);
    bus.Subscribe<Ping>([&](const Ping&) { ++second; }, &second);
    bus.Subscribe<Ping>([&](const Ping&) { ++unowned; });
    bus.Post(Ping{});
    bus.Post(Pong{});
    bus.Unsubscribe(&first);
    bus.Unsubscribe(&first);
    bus.Unsubscribe(nullptr);
    bus.ProcessQueuedEvents();
    bus.Publish(Ping{});
    bus.Publish(Pong{});
    EXPECT_EQ(first, 0);
    EXPECT_EQ(second, 2);
    EXPECT_EQ(unowned, 2);
}

TEST(EventBusTest, UnsubscribeDuringDispatchSkipsRemovedCallbacks) {
    for (bool queued : {false, true}) {
        EventBus bus;
        int first = 0, removed = 0, remaining = 0;
        bus.Subscribe<Ping>([&](const Ping&) {
            ++first;
            bus.Unsubscribe(&first); // The currently executing callback must stay alive.
            bus.Unsubscribe(&removed);
        }, &first);
        bus.Subscribe<Ping>([&](const Ping&) { ++removed; }, &removed);
        bus.Subscribe<Ping>([&](const Ping&) { ++remaining; }, &remaining);
        if (queued) {
            bus.Post(Ping{});
            bus.ProcessQueuedEvents();
        } else {
            bus.Publish(Ping{});
        }
        bus.Publish(Ping{});
        EXPECT_EQ(first, 1);
        EXPECT_EQ(removed, 0);
        EXPECT_EQ(remaining, 2);
    }
}

TEST(EventBusTest, NewSubscriptionsBeginOnNextDispatchIncludingNestedPublish) {
    EventBus bus;
    int first = 0, added = 0;
    bus.Subscribe<Ping>([&](const Ping&) {
        ++first;
        bus.Unsubscribe(&first);
        bus.Subscribe<Ping>([&](const Ping&) { ++added; }, &added);
        bus.Publish(Ping{});
    }, &first);
    bus.Publish(Ping{});
    EXPECT_EQ(first, 1);
    EXPECT_EQ(added, 1);
    bus.Publish(Ping{});
    EXPECT_EQ(added, 2);
}

TEST(EventBusTest, ClearingDuringDispatchInvalidatesSnapshotAndAllowsResubscription) {
    EventBus bus;
    int first = 0, removed = 0, added = 0;
    bus.Subscribe<Ping>([&](const Ping&) {
        ++first;
        bus.ClearAll();
        bus.Subscribe<Ping>([&](const Ping&) { ++added; });
    });
    bus.Subscribe<Ping>([&](const Ping&) { ++removed; });
    bus.Publish(Ping{});
    EXPECT_EQ(first, 1);
    EXPECT_EQ(removed, 0);
    EXPECT_EQ(added, 0);
    bus.Publish(Ping{});
    EXPECT_EQ(added, 1);
}

class EventLifecycleTest : public ::testing::Test {
protected:
    void SetUp() override {
        LevelHandler::Create();
        LevelHandler::GetSingleton().Load(nullptr);
        LevelHandler::GetSingleton().Flatten(0);
    }
    void TearDown() override {
        // No ClearAll(): each world/system must clean up its own callbacks.
        Events::ProcessQueuedEvents();
        LevelHandler::Destroy();
    }
    void ExpectNoWorldCallbacks() {
        GetLevelBoundsQuery bounds;
        Events::Publish(bounds);
        EXPECT_FLOAT_EQ(bounds.sizeX, 0);
        EXPECT_FLOAT_EQ(bounds.sizeZ, 0);
        PointCollisionQuery point(30, 2, 30, CollisionLayer::ALL_TANKS);
        Events::Publish(point);
        EXPECT_FALSE(point.result);
        SphereCollisionQuery sphere(30, 2, 30, 1, CollisionLayer::ALL_TANKS);
        Events::Publish(sphere);
        EXPECT_TRUE(sphere.results.empty());
        Bullet bullet;
        Events::Publish(BulletTimeoutEvent(&bullet, 100));
        Events::Publish(BulletOutOfBoundsEvent(&bullet, 0, 0, 0));
        EXPECT_TRUE(bullet.IsAlive());
        Events::Publish(Effect());
    }
};

TEST_F(EventLifecycleTest, RepeatedWorldShutdownAndDestructionLeaveNoCallbacks) {
    Observer observer;
    Events::Subscribe<CreateFXEvent>([&](const CreateFXEvent&) { ++observer.calls; }, &observer);
    for (int cycle = 0; cycle < 10; ++cycle) {
        auto world = std::make_unique<GameWorld>();
        world->Initialize();
        world->Initialize(); // Must not duplicate combat or FX subscriptions.
        Events::Publish(Effect());
        EXPECT_EQ(world->GetFX().size(), 1u);
        Tank* target = world->CreateTank();
        target->identity = TankIdentity::Enemy(2);
        target->health = 100;
        Bullet bullet(TankIdentity::Enemy(1), 20, TankType::TYPE_RED, TankType::TYPE_GREY,
                      0, 0, Color(), Color(), 30, 2, 30, 0, 0, 0);
        Events::Publish(BulletCollisionEvent(&bullet, target, 30, 2, 30));
        EXPECT_FLOAT_EQ(target->health, 80);
        EXPECT_FALSE(bullet.IsAlive());
        Events::ProcessQueuedEvents();
        if (cycle % 2 == 0) {
            world->Shutdown();
            world->Shutdown();
            const size_t fxCount = world->GetFX().size();
            ExpectNoWorldCallbacks();
            EXPECT_EQ(world->GetFX().size(), fxCount);
        }
        // Odd cycles rely only on the destructor.
        Bullet queuedBullet;
        Events::Post(BulletTimeoutEvent(&queuedBullet, 100));
        Events::Post(GetLevelBoundsQuery{});
        Events::Post(SphereCollisionQuery(30, 2, 30, 1, CollisionLayer::ALL_TANKS));
        Events::Post(Effect());
        world.reset();
        int previous = observer.calls;
        Events::ProcessQueuedEvents();
        EXPECT_EQ(observer.calls, previous + 1); // Shutdown must not clear other subscriptions/queue.
        EXPECT_TRUE(queuedBullet.IsAlive());
        ExpectNoWorldCallbacks();
        EXPECT_EQ(observer.calls, previous + 2);
    }
}

TEST_F(EventLifecycleTest, WorldCanInitializeAgainAfterShutdown) {
    GameWorld world;
    for (int cycle = 0; cycle < 3; ++cycle) {
        world.Initialize();
        GetLevelBoundsQuery bounds;
        Events::Publish(bounds);
        EXPECT_FLOAT_EQ(bounds.sizeX, 128);
        Bullet bullet;
        Events::Publish(BulletTimeoutEvent(&bullet, 100));
        EXPECT_FALSE(bullet.IsAlive());
        size_t count = world.GetFX().size();
        Events::Publish(Effect());
        EXPECT_EQ(world.GetFX().size(), count + 1);
        world.Shutdown();
        ExpectNoWorldCallbacks();
        EXPECT_EQ(world.GetFX().size(), count + 1);
    }
}

TEST_F(EventLifecycleTest, ShuttingDownOneWorldPreservesAnotherWorld) {
    GameWorld remaining;
    remaining.Initialize();
    {
        GameWorld removed;
        removed.Initialize();
        removed.Shutdown();
        Events::Publish(Effect());
        EXPECT_TRUE(removed.GetFX().empty());
        EXPECT_EQ(remaining.GetFX().size(), 1u);
    }
    Events::Post(Effect());
    Events::ProcessQueuedEvents();
    EXPECT_EQ(remaining.GetFX().size(), 2u);
    GetLevelBoundsQuery bounds;
    Events::Publish(bounds);
    EXPECT_FLOAT_EQ(bounds.sizeX, 128);
    Bullet bullet;
    Events::Publish(BulletTimeoutEvent(&bullet, 100));
    EXPECT_FALSE(bullet.IsAlive());
}

TEST_F(EventLifecycleTest, StandaloneSystemsUnsubscribeOnDestruction) {
    {
        CollisionSystem collision;
        CombatSystem combat;
        collision.Initialize();
        combat.Initialize();
        GetLevelBoundsQuery bounds;
        Events::Publish(bounds);
        EXPECT_FLOAT_EQ(bounds.sizeX, 128);
        Bullet bullet;
        Events::Publish(BulletTimeoutEvent(&bullet, 100));
        EXPECT_FALSE(bullet.IsAlive());
    }
    ExpectNoWorldCallbacks();
}
