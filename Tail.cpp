#include "Entity.h"
#include "Tail.h"
#include "EntityList.h"

Texture* Tail::_tailTexture = nullptr;

void Tail::_ParseSprites(std::string line) {
	_animatedSprite.ParseSprites(line, _tailTexture);
}

Tail::Tail() {
	_renderPriority = 0;
	_bounceSpeed = 0.23f;

	_touchedEntity = nullptr;

	isPassThroughable = true;
}

Tail::~Tail() {}

void Tail::ParseData(
	std::string dataPath,
	Texture*& texture,
	std::vector<std::string> extraData)
{
	if (_tailTexture == nullptr) {
		_tailTexture = texture;
	}
	Entity::ParseData(dataPath, texture, extraData);
}

void Tail::HandleStates() {}

void Tail::HandleCollisionResult(
	LPCOLLISIONEVENT result,
	D3DXVECTOR2& minTime,
	D3DXVECTOR2& offset,
	D3DXVECTOR2& normal,
	D3DXVECTOR2& relativeDistance) {}

void Tail::HandleOverlap(Entity* entity) {
	//Handle objects collision event with tail object when player hit attack button in raccoon form
	switch (entity->GetObjectType()) {
	case GameObjectType::GAMEOBJECT_TYPE_GOOMBA:
		_touchedEntity = entity;
		break;
	}

	switch (entity->GetObjectType()) {
	case GameObjectType::GAMEOBJECT_TYPE_GOOMBA:
	case GameObjectType::GAMEOBJECT_TYPE_PARAGOOMBA:
	{
		Goomba* goomba = dynamic_cast<Goomba*>(entity);
		goomba->animationName = "Walk";
		goomba->SetHealth(0);
		goomba->SetScale({ 1.0f, -1.0f });
		goomba->SetVelocity({ 0.0f, -_bounceSpeed });
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_COIN:
	{
		Coin* coin = dynamic_cast<Coin*>(entity);
		//Is brick
		if (coin->GetHealth() == 3) {
			coin->SetHealth(-1);
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_QUESTIONBLOCK:
	{
		QuestionBlock* questionBlock = dynamic_cast<QuestionBlock*>(entity);
		questionBlock->TakeDamage();
	}
	break;
	}
}

void Tail::Update(
	DWORD deltaTime,
	std::vector<Entity*>* collidableEntities,
	std::vector<Entity*>* collidableTiles,
	Grid* grid)
{
	if (_touchedEntity != nullptr && GetTickCount64() % 100 == 0) {
		_touchedEntity = nullptr;
	}

	Entity::Update(deltaTime, collidableEntities, collidableTiles, grid);
}

void Tail::Render() {
	_animatedSprite.PlaySpriteAnimation("Tail", _position);

	if (_touchedEntity != nullptr) {
		_animatedSprite.PlaySpriteAnimation("Spark", _touchedEntity->GetPosition());
	}
}

void Tail::Release() {
	_animatedSprite.Release();
	_tailTexture = nullptr;
}