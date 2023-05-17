#pragma once

#define NOMINMAX

#include "Hitbox.h"

#include <fstream>
#include <algorithm>

#include <d3dx10.h>

class GameObject;
class Entity;

struct CollisionEvent;
typedef CollisionEvent* LPCOLLISIONEVENT;
struct CollisionEvent {
	GameObject* gameOject;
	Entity* entity;
	D3DXVECTOR2 normal;
	D3DXVECTOR2 distance;
	float time;

	CollisionEvent(GameObject*& obj, D3DXVECTOR2 norm, D3DXVECTOR2 dist, float t) {
		gameOject = obj;
		normal = norm;
		distance = dist;
		time = t;
	}

	CollisionEvent(Entity*& ent, D3DXVECTOR2 norm, D3DXVECTOR2 dist, float t) {
		entity = ent;
		normal = norm;
		distance = dist;
		time = t;
	}

	static bool CompareCollisionEvent(const LPCOLLISIONEVENT& a, const LPCOLLISIONEVENT& b) {
		return a->time < b->time;
	}
};

class GameObject {
public:
	enum class GameObjectType {
		//Players
		GAMEOBJECT_TYPE_MARIO = 0
	};

protected:
	bool _isActive;

	GameObjectType _objectType;

	Hitbox _hitbox;

	DWORD _deltaTime;

	D3DXVECTOR2 _velocity;
	D3DXVECTOR2 _distance;
	D3DXVECTOR2 _normal;

	D3DXVECTOR2 _position;
	D3DXVECTOR2 _rotation;
	D3DXVECTOR2 _translation;
	D3DXVECTOR2 _scale;

public:
	//If true, the entity is removed from the container
	//But its resources will not be released
	//Useful for when an object needs to know when another object is ready to be removed
	bool flaggedForRemoval;
	//Pass throughable entities are entities that
	//Other entities can pass through without blocking their velocity

	GameObject();
	virtual ~GameObject();

	//Is this Java?
	void SetActive(bool);
	bool IsActive() const;

	bool IsOverlapped(GameObject*) const;

	void SetOjectType(GameObjectType);
	GameObjectType GetObjectType() const;

	virtual RECTF GetBoundingBox(int = 0) const;
	float GetBoxWidth(int = 0) const;
	float GetBoxHeight(int = 0) const;

	virtual void SetVelocity(D3DXVECTOR2);
	D3DXVECTOR2 GetVelocity() const;
	virtual void SetDistance(D3DXVECTOR2);
	D3DXVECTOR2 GetDistance() const;
	virtual void SetNormal(D3DXVECTOR2);
	D3DXVECTOR2 GetNormal() const;
	virtual void SetPosition(D3DXVECTOR2);
	D3DXVECTOR2 GetPosition() const;
	virtual void SetRotation(D3DXVECTOR2);
	D3DXVECTOR2 GetRotation() const;
	virtual void SetTranslation(D3DXVECTOR2);
	D3DXVECTOR2 GetTranslation() const;
	virtual void SetScale(D3DXVECTOR2);
	D3DXVECTOR2 GetScale() const;

	//Moving object, static object, distance of moving object, normal, time
	void SweptAABB(RECTF, RECTF, D3DXVECTOR2, D3DXVECTOR2&, float&);
	//Filter the collision between objects based on the shortest time
	void FilterCollision(const std::vector<LPCOLLISIONEVENT>&, std::vector<LPCOLLISIONEVENT>&, D3DXVECTOR2&, D3DXVECTOR2&, D3DXVECTOR2&);

	virtual void Update(DWORD, std::vector<GameObject*>* = nullptr);
	virtual void Render();

	virtual void Release() = 0;
};