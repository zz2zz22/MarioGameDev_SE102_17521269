#include "Entity.h"
#include "Fireball.h"
#include "EntityList.h"

Texture* Fireball::_fireballTexture = nullptr;

void Fireball::_ParseSprites(std::string line) {
	_animatedSprite.ParseSprites(line, _fireballTexture);
}

Fireball::Fireball() {
	_renderPriority = 2;
	_removeTime = 300;

	_travelSpeed = 0.0012f;
	_bounceSpeed = 0.2f;
	_gravity = 0.002f;

	_aliveTime = 8000;

	isPassThroughable = true;
	StartAliveTimer();
}

Fireball::~Fireball() {}

bool Fireball::IsAlive() const {
	return _aliveStart != 0;
}

void Fireball::StartAliveTimer() {
	_aliveStart = static_cast<DWORD>(GetTickCount64());
}

RECTF Fireball::GetBoundingBox(int index) const {
	return _health <= 0 ? RECTF() : GameObject::GetBoundingBox();
}

void Fireball::ParseData(
	std::string dataPath,
	Texture*& texture,
	std::vector<std::string> extraData)
{
	if (_fireballTexture == nullptr) {
		_fireballTexture = texture;
	}
	Entity::ParseData(dataPath, texture, extraData);

	_runSpeed = _objectType == GameObjectType::GAMEOBJECT_TYPE_PLAYERFIREBALL ? 0.2f : 0.05f;
}

void Fireball::HandleStates() {
	_state = static_cast<_State>(_health);

	switch (_state) {
	case _State::BOUNCE:
		_velocity.x = _runSpeed * _normal.x;
		break;
	case _State::EXPLODE:
		_velocity = { 0.0f, 0.0f };

		if (_health == 0 && !IsRemoved()) {
			StartRemoveTimer();
		}
		break;
	}
}

void Fireball::HandleCollisionResult(
	LPCOLLISIONEVENT result,
	D3DXVECTOR2& minTime,
	D3DXVECTOR2& offset,
	D3DXVECTOR2& normal,
	D3DXVECTOR2& relativeDistance)
{
	Entity* eventEntity = result->entity;
	D3DXVECTOR2 eventNormal = result->normal;

	switch (_objectType) {
	case GameObjectType::GAMEOBJECT_TYPE_PLAYERFIREBALL:
		if (eventNormal.y == -1.0f) {
			_velocity.y = -_bounceSpeed;
		}

		switch (eventEntity->GetObjectType()) {
		case GameObjectType::GAMEOBJECT_TYPE_GOOMBA:
		case GameObjectType::GAMEOBJECT_TYPE_PARAGOOMBA:
			eventEntity->SetHealth(0);
			eventEntity->SetScale({ 1.0f, -1.0f });
			eventEntity->SetVelocity({ 0.0f, -_bounceSpeed });
			break;
		}
	case GameObjectType::GAMEOBJECT_TYPE_COIN:
	{
		Coin* coin = dynamic_cast<Coin*>(eventEntity);
		if (coin->GetHealth() == 3 && eventNormal.x != 0.0f) {
			coin->SetHealth(-1);
			TakeDamage();
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_QUESTIONBLOCK:
	{
		QuestionBlock* questionBlock = dynamic_cast<QuestionBlock*>(eventEntity);
		if (eventNormal.x != 0.0f) {
			questionBlock->TakeDamage();
			TakeDamage();
		}
	}
	break;
	}
}

void Fireball::Update(
	DWORD deltaTime,
	std::vector<Entity*>* collidableEntities,
	std::vector<Entity*>* collidableTiles,
	Grid* grid)
{
	if (IsAlive() && GetTickCount64() - _aliveStart > _aliveTime) {
		_health = -1;
		_aliveStart = 0;
	}

	HandleStates();
	Entity::Update(deltaTime, collidableEntities, collidableTiles, grid);
}

void Fireball::Render() {
	switch (_state) {
	case _State::BOUNCE:
		_animatedSprite.PlaySpriteAnimation("Fireball", _position, { _normal.x, 1.0f });
		break;
	case _State::EXPLODE:
		_animatedSprite.PlaySpriteAnimation("Explode", _position);
		break;
	}
}

void Fireball::Release() {
	_animatedSprite.Release();
	_fireballTexture = nullptr;
}