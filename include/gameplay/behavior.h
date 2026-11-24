#pragma once

class Entity;

class Behavior{
public:
void setOwner(Entity* entity) { owner = entity; }
Entity* getOwner() const { return owner; }
virtual ~Behavior()= default;

virtual void start(){}
virtual void update(float dt){}
virtual void cleanup(){}

protected:
Entity* owner = nullptr;

};