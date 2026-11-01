#pragma once

class Entity;
class Renderer;

class Component {

  public:
    virtual ~Component();

    virtual void update(float deltaTime);

    virtual void render(Renderer& renderer);

    Entity* getOwner() const;

  private:
    void setOwner(Entity& entity);

    Entity* owner = nullptr;

    friend class Entity;
};
