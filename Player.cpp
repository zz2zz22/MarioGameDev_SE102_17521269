#include "Device.h"
#include "SceneManager.h"
#include "Player.h"
#include "IdleState.h"
#include "EntityList.h"

Texture* Player::_playerTexture = nullptr;
std::vector<GameObject::GameObjectType> Player::_bonusItems;

void Player::_ParseSprites(std::string line) {
	_animatedSprite.ParseSprites(line, _playerTexture);
}

void Player::_HandleCurrencies() {
	if (_lives > _MAX_LIVES) {
		_lives = _MAX_LIVES;
	}

	if (_coins > _MAX_COINS) {
		_coins = _MAX_COINS;
	}

	if (_score > _MAX_SCORE) {
		_score = _MAX_SCORE;
	}
}

void Player::_HanldeStageEnd() {
	if (_triggeredStageEnd) {
		if (!_hasBossItem) {
			_acceleration = _MIN_ACCEL;
		}

		_isHolding = false;

		if (_sceneRemainingTime > 0) {
			for (unsigned int i = 0; i < _sceneRemainingTime; ++i) {
				_score += 50;
			}
			_sceneRemainingTime = 0;
		}

	}
}

void Player::_HandleBonusItems() {
	if (_bonusItems.empty()) {
		return;
	}

	unsigned int shroomCards = 0;
	unsigned int flowerCards = 0;
	unsigned int starCards = 0;


	const unsigned int MAX_ITEMS = 3;
	if (_bonusItems.size() == MAX_ITEMS) {
		if (shroomCards == MAX_ITEMS) {
			_lives += 2;
		}
		else if (flowerCards == MAX_ITEMS) {
			_lives += 3;
		}
		else if (starCards == MAX_ITEMS) {
			_lives += 5;
		}
		else {
			_lives += 1;
		}

		_bonusItems.clear();
	}
}

void Player::_HandleMovementMap() {
	if (abs(_position.x - _lastPos.x) >= _MAX_TRAVEL_DISTANCE || abs(_position.y - _lastPos.y) >= _MAX_TRAVEL_DISTANCE) {
		_velocity = { 0.0f, 0.0f };
		_lastPos = _position;
		if (_mapNodePos.x != 0.0f && _mapNodePos.y != 0.0f) {
			_position = _mapNodePos;
		}
	}
}

void Player::_HandleMovementGame() {
	if (_isOnGround) {
		_gravity = 0.0025f;
	}

	//Variable jump height	
	if (Device::IsKeyDown(DIK_K)) {
		if (_gravity > _MAX_GRAVITY) {
			_gravity -= 0.0005f;
		}
		else if (_gravity <= _MAX_GRAVITY) {
			_gravity = _MAX_GRAVITY;
		}
	}
	else {
		if (_gravity < 0.0025f) {
			_gravity += 0.0005f;
		}
	}

	//Skid
	if (_acceleration < _ACCEL_THRESHOLD && _velocity.x != 0.0f) {
		if (_normal.x == -1) {
			if (Device::IsKeyDown(DIK_D)) {
				_acceleration = 0.0499f;
			}
		}
		else if (_normal.x == 1) {
			if (Device::IsKeyDown(DIK_A)) {
				_acceleration = 0.0499f;
			}
		}
	}

	if (Device::IsKeyDown(DIK_A)) {
		MoveLeft();
	}
	else if (Device::IsKeyDown(DIK_D)) {
		MoveRight();
	}
	else {
		//Slippery feel when the player stops
		if (_acceleration <= 0.5f) {
			_velocity.x = 0.0f;
			_acceleration = 0.5f;
		}
		else if (_acceleration > 0.5f) {
			_acceleration -= 0.06f;
		}
	}

	if (Device::IsKeyDown(DIK_A) || Device::IsKeyDown(DIK_D)) {
		//GOTTA GO FAAAST
		if (Device::IsKeyDown(DIK_J) && (_isOnGround || !IsFlying())) {
			if (_acceleration < _MAX_ACCEL) {
				_acceleration += 0.03f;
			}
		}
		else {
			if (_acceleration < _MIN_ACCEL) {
				_acceleration += 0.03f;
			}
			else if (_acceleration > _MIN_ACCEL) {
				_acceleration -= 0.02f;
			}
		}
	}

	//Float a bit longer when flying
	if (_acceleration >= _ACCEL_THRESHOLD) {
		_gravity = 0.0013f;
	}
}

Player::Player() {
	_scale = D3DXVECTOR2(-1.0f, 1.0f);
	_renderPriority = 0;

	_runSpeed = 0.09f;
	_jumpSpeed = 0.327f;
	_bounceSpeed = 0.4f;
	_gravity = 0.0025f;
	_acceleration = 0.5f;

	_health = 1;

	_lives = 3;
	_coins = 0;
	_score = 0;

	_fireballsCount = 0;

	_upVector = 1.0f;

	_heldEntity = nullptr;

	_flyTime = 6000;
	_inPipeTime = 2000;
	_attackTime = 150;
	_fireballCoolDownTime = 2500;
	_invulnerableTime = 1000;

	_playerState = new IdleState(this);

	_bonusItems.reserve(3);
}

Player::~Player() {}

unsigned int Player::GetNextSceneID() const {
	return _nextSceneID;
}

RECTF Player::GetBoundingBox(int index) const {
	return GameObject::GetBoundingBox(_health >= 2 && !isInMap && !_isCrouching);
}

Entity* Player::GetHeldEntity() const {
	return _heldEntity;
}

void Player::GetSceneRemainingTime(unsigned int sceneTime) {
	_sceneRemainingTime = sceneTime;
}

void Player::SetUpVector(float upVector) {
	if (upVector == 0.0f) {
		return;
	}

	_upVector = upVector;
}

bool Player::TriggeredStageEnd() const {
	return _triggeredStageEnd;
}

bool Player::WentIntoPipe() const {
	return _wentIntoPipe;
}

bool Player::IsFlying() const {
	return _flyStart != 0;
}

bool Player::IsInPipe() const {
	return _inPipeStart != 0;
}

bool Player::IsAttacking() const {
	return _attackStart != 0;
}

bool Player::IsOnFireballCoolDown() const {
	return _fireballCoolDownStart != 0;
}

bool Player::IsInvulnerable() const {
	return _invulnerableStart != 0;
}

void Player::StartFlyTimer() {
	_flyStart = static_cast<DWORD>(GetTickCount64());
}

void Player::StartInPipeTimer() {
	_inPipeStart = static_cast<DWORD>(GetTickCount64());
}

void Player::StartAttackTimer() {
	_attackStart = static_cast<DWORD>(GetTickCount64());
}

void Player::StartFireballCoolDownTimer() {
	_fireballCoolDownStart = static_cast<DWORD>(GetTickCount64());
}

void Player::StartInvulnerableTimer() {
	_invulnerableStart = static_cast<DWORD>(GetTickCount64());
}

void Player::HandleStates() {
	if (isInMap) {
		_HandleMovementMap();
	}
	else {
		_HandleMovementGame();
	}

	PlayerState* currentState = _playerState->HandleStates();
	if (currentState != nullptr) {
		delete _playerState;
		_playerState = currentState;
	}
}

void Player::OnKeyUpMap(int keyCode) {
	//Stub
}

void Player::OnKeyUpGame(int keyCode) {
	switch (keyCode) {
	case DIK_S:
		_isCrouching = false;

		if (_health > 1 && _isOnGround && !IsInPipe()) {
			_isOnGround = false;
			_position.y = ceil(_position.y - _CROUCH_HEIGHT_ADJUST);
		}
		break;
	case DIK_J:
		_isHolding = false;
		break;
	}
}

void Player::OnKeyDownMap(int keyCode) {
	const float MAP_RUN_SPEED = 0.08f;
	switch (keyCode) {
	case DIK_W:
		_velocity.y = -MAP_RUN_SPEED;
		break;
	case DIK_A:
		_velocity.x = -MAP_RUN_SPEED;
		break;
	case DIK_S:
		_velocity.y = MAP_RUN_SPEED;
		break;
	case DIK_D:
		_velocity.x = MAP_RUN_SPEED;
		break;
	}
}

void Player::OnKeyDownGame(int keyCode) {
	switch (keyCode) {
	case DIK_A:
		_normal.x = -1.0f;
		break;
	case DIK_D:
		_normal.x = 1.0f;
		break;
	case DIK_S:
		_isCrouching = true;

		if (_health > 1 && _isOnGround && !IsInPipe()) {
			_position.y = ceil(_position.y + _CROUCH_HEIGHT_ADJUST);
		}
		break;
	case DIK_J:
		_isHolding = true;
		//Fireball attack
		if (_health == 3 && !_isCrouching) {
			if (_fireballsCount < _FIREBALLS_LIMIT) {
				SceneManager::GetInstance()->GetCurrentScene()->AddEntityToScene(SpawnFireball());
				++_fireballsCount;

				if (_fireballsCount == _FIREBALLS_LIMIT) {
					StartFireballCoolDownTimer();
				}
			}
		}

		//Tail attack
		if (_health == 4 && !IsAttacking()) {
			StartAttackTimer();
		}
		break;
	case DIK_K:
		if (!_isCrouching)
		{
			SlowFall();
			RunFly();
			Jump();
		}
		break;
	}
}

void Player::ParseData(
	std::string dataPath,
	Texture*& texture,
	std::vector<std::string> extraData)
{
	if (_playerTexture == nullptr) {
		_playerTexture = texture;
	}
	Entity::ParseData(dataPath, texture, extraData);
}

void Player::TakeDamage() {
	if (!IsInvulnerable()) {
		_originalVel = { _velocity.x, _isOnGround ? _velocity.y : -_bounceSpeed };

		StartInvulnerableTimer();

		if (_health > 2) {
			_health = 2;
		}
		else {
			--_health;
			if (_health < 1) {
				--_lives;
			}
		}
	}
}

void Player::MoveLeft() {
	_scale = D3DXVECTOR2(1.0f, 1.0f);
	_velocity.x = -_runSpeed * _acceleration;
}

void Player::MoveRight() {
	_scale = D3DXVECTOR2(-1.0f, 1.0f);
	_velocity.x = _runSpeed * _acceleration;
}

void Player::Jump() {
	if (_isOnGround) {
		_velocity.y = -_jumpSpeed;
		_isOnGround = false;
	}
}

void Player::RunFly() {
	if (_health == 4) {
		if (_acceleration >= _ACCEL_THRESHOLD || IsFlying()) {
			if (_isOnGround && !IsFlying()) {
				_isOnGround = false;
				StartFlyTimer();
			}

			const float FLOAT_MODIFIER = 0.66f;
			_velocity.y = -_jumpSpeed * FLOAT_MODIFIER;
		}
	}
}

void Player::SlowFall() {
	if (_health == 4 && !_isOnGround) {
		const float SLOW_MODIFIER = 0.2f;
		_velocity.y *= SLOW_MODIFIER;
	}
}

Fireball* Player::SpawnFireball() {
	Fireball* fireball = dynamic_cast<Fireball*>(
		SceneManager::GetInstance()->GetCurrentScene()->CreateEntityFromData(
			_extraData.at(0),
			_extraData.at(1),
			_extraData.at(2)
		)
		);
	const float OFFSET = 10.0f;
	fireball->SetNormal({ _normal.x, fireball->GetNormal().y });
	fireball->SetPosition({ _position.x, _position.y + OFFSET });
	return fireball;
}

void Player::HandleCollisionResult(
	LPCOLLISIONEVENT result,
	D3DXVECTOR2& minTime,
	D3DXVECTOR2& offset,
	D3DXVECTOR2& normal,
	D3DXVECTOR2& relativeDistance)
{
	Entity* eventEntity = result->entity;
	D3DXVECTOR2 eventNormal = result->normal;

	if (IsInPipe() || IsInvulnerable()) {
		if (IsInPipe()) {
			minTime = { 1.0f, 1.0f };
			offset = normal = { 0.0f, 0.0f };
		}
		return;
	}

	switch (eventEntity->GetObjectType()) {
	case GameObjectType::GAMEOBJECT_TYPE_PORTAL:
	case GameObjectType::GAMEOBJECT_TYPE_QUESTIONBLOCK:
	case GameObjectType::GAMEOBJECT_TYPE_TILE:
	case GameObjectType::GAMEOBJECT_TYPE_ONEWAYPLATFORM:
		if (eventNormal.y == -1.0f) {
			_isOnGround = true;
		}
		break;
	case GameObjectType::GAMEOBJECT_TYPE_COIN:
		//Is brick
		if (eventEntity->GetHealth() == 3) {
			_isOnGround = true;
		}
		break;
	case GameObjectType::GAMEOBJECT_TYPE_SHINYBRICK:
		//Is coin
		if (eventEntity->GetHealth() != 3) {
			_isOnGround = true;
		}
		break;
	}

	switch (eventEntity->GetObjectType()) {
		//----------------------------------------------------------------------------
		//NPCs
		//----------------------------------------------------------------------------
	case GameObjectType::GAMEOBJECT_TYPE_GOOMBA:
	case GameObjectType::GAMEOBJECT_TYPE_PARAGOOMBA:
	{
		Goomba* goomba = dynamic_cast<Goomba*>(eventEntity);
		if (eventNormal.y == -1.0f) {
			if (goomba->GetHealth() > 0) {
				goomba->TakeDamage();
				_velocity.y = -_bounceSpeed;
			}
		}
		else if (eventNormal.y == 1.0f || eventNormal.x != 0.0f) {
			if (goomba->GetHealth() > 0) {
				TakeDamage();
				_velocity.y = -_bounceSpeed;
			}
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_KOOPA:
	case GameObjectType::GAMEOBJECT_TYPE_PARAKOOPA:
	{
		Koopa* koopa = dynamic_cast<Koopa*>(eventEntity);
		if (eventNormal.y == -1.0f) {
			if (koopa->GetHealth() > 0) {
				_velocity.y = -_bounceSpeed;
			}

			if (koopa->GetHealth() != 1) {
				koopa->TakeDamage();
			}
			else if (koopa->GetHealth() == 1) {
				koopa->SetHealth(2);
			}
		}
		else if (eventNormal.y == 1.0f) {
			if (koopa->GetHealth() > 1 && koopa->GetHealth() != 2) {
				TakeDamage();
				_velocity.y = -_bounceSpeed;
			}
			else if (koopa->GetHealth() == 2) {
				koopa->TakeDamage();
			}
		}
		else if (eventNormal.x != 0.0f) {
			if (koopa->GetHealth() == 2) {
				if (_isHolding) {
					if (_heldEntity == nullptr) {
						_heldEntity = koopa;
						koopa->isBeingHeld = true;
					}
				}
				else {
					_isNextToShell = true;
					koopa->TakeDamage();
					koopa->SetNormal({ -_normal.x, koopa->GetNormal().y });
				}
			}
			else {
				TakeDamage();
				_velocity.y = -_bounceSpeed;
			}
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_PIRANHAPLANT:
	case GameObjectType::GAMEOBJECT_TYPE_VENUSPLANT:
	{
		PiranaPlant* piranaPlant = dynamic_cast<PiranaPlant*>(eventEntity);
		if (piranaPlant->GetHealth() >= 0) {
			TakeDamage();
			_velocity.y = -_bounceSpeed;
		}
	}
	break;
	//----------------------------------------------------------------------------
	//NPCs
	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	//PROJECTILES
	//----------------------------------------------------------------------------
	case GameObjectType::GAMEOBJECT_TYPE_VENUSFIREBALL:
		TakeDamage();
		break;

		//----------------------------------------------------------------------------
		//PROJECTILES
		//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	//SPECIAL ENTITIES
	//----------------------------------------------------------------------------
	case GameObjectType::GAMEOBJECT_TYPE_PORTAL:
	{
		Portal* portal = dynamic_cast<Portal*>(eventEntity);
		if (portal->GetExtraData().size() == 1) {
			_nextSceneID = portal->GetSceneID();
			_mapNodePos = portal->GetPosition();
		}
		else {
			if (Device::IsKeyDown(DIK_S) || Device::IsKeyDown(DIK_W)) {
				if (eventNormal.y == 1.0f && Device::IsKeyDown(DIK_W)) {
					_normal.y = -1.0f;
				}
				else if (eventNormal.y == -1.0f && Device::IsKeyDown(DIK_S)) {
					_normal.y = 1.0f;
				}

				if (!IsInPipe()) {
					StartInPipeTimer();
					_destination = portal->GetDestination();
				}
			}
		}
	}
	break;
	//----------------------------------------------------------------------------
	//SPECIAL ENTITIES
	//----------------------------------------------------------------------------


		//----------------------------------------------------------------------------
		//ITEMS
		//----------------------------------------------------------------------------
	case GameObjectType::GAMEOBJECT_TYPE_REDMUSHROOM:
	case GameObjectType::GAMEOBJECT_TYPE_GREENMUSHROOM:
	{
		Mushroom* mushroom = dynamic_cast<Mushroom*>(eventEntity);
		mushroom->TakeDamage();
		switch (mushroom->GetObjectType()) {
		case GameObjectType::GAMEOBJECT_TYPE_REDMUSHROOM:
			if (_health == 1) {
				_health = 2;
				_position.y -= GetBoxHeight();

				if (!IsInvulnerable()) {
					_originalVel = _velocity;

					StartInvulnerableTimer();
				}
			}
			break;
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_LEAF:
	{
		Leaf* leaf = dynamic_cast<Leaf*>(eventEntity);
		leaf->TakeDamage();

		if (_health != 4) {
			if (_health == 1) {
				_health = 2;
			}
			else {
				_health = 4;
			}

			if (!IsInvulnerable()) {
				_originalVel = _velocity;

				StartInvulnerableTimer();
			}
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_FLOWER:
	{
		Flower* flower = dynamic_cast<Flower*>(eventEntity);
		flower->TakeDamage();

		if (_health != 3) {
			if (_health == 1) {
				_health = 2;
			}
			else {
				_health = 3;
			}

			if (!IsInvulnerable()) {
				_originalVel = _velocity;

				StartInvulnerableTimer();
			}
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_COIN:
	{
		Coin* coin = dynamic_cast<Coin*>(eventEntity);
		if (coin->GetHealth() == 1) {
			coin->TakeDamage();
		}
		//Is brick
		else if (coin->GetHealth() == 3) {
			if (eventNormal.y == 1.0f) {
				coin->SetHealth(-1);
			}
		}
	}
	break;
	//----------------------------------------------------------------------------
	//ITEMS
	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	//ANIMATED BLOCKS
	//----------------------------------------------------------------------------
	case GameObjectType::GAMEOBJECT_TYPE_QUESTIONBLOCK:
	{
		QuestionBlock* questionBlock = dynamic_cast<QuestionBlock*>(eventEntity);
		if (eventNormal.y == 1.0f) {
			questionBlock->TakeDamage();
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_SHINYBRICK:
	{
		ShinyBrick* shinyBrick = dynamic_cast<ShinyBrick*>(eventEntity);
		if (shinyBrick->GetHealth() == 2) {
			if (eventNormal.y == 1.0f) {
				if (shinyBrick->GetExtraData().size() == 3) {
					if (_health > 1) {
						shinyBrick->SetHealth(-1);
					}
				}
				else {
					shinyBrick->TakeDamage();
				}
			}
		}
		//Is coin
		else if (shinyBrick->GetHealth() == 3) {
			shinyBrick->SetHealth(-2);

			_coins += 1;
		}
	}
	break;
	case GameObjectType::GAMEOBJECT_TYPE_PBLOCK:
	{
		PBlock* pBlock = dynamic_cast<PBlock*>(eventEntity);
		if (eventNormal.y == -1.0f) {
			pBlock->TakeDamage();
			_velocity.y = -_bounceSpeed;
		}
	}
	break;
	//----------------------------------------------------------------------------
	//ANIMATED BLOCKS
	//----------------------------------------------------------------------------
	}
}

void Player::HandleOverlap(Entity* entity) {
	switch (entity->GetObjectType()) {
	case GameObjectType::GAMEOBJECT_TYPE_BONUSITEM:
	{
		BonusItem* bonusItem = dynamic_cast<BonusItem*>(entity);
		if (bonusItem->GetHealth() == 2) {
			_bonusItems.emplace_back(bonusItem->GetCurrentItem());
			bonusItem->TakeDamage();
		}

		_triggeredStageEnd = true;
	}
	break;
	}
}

void Player::Update(
	DWORD deltaTime,
	std::vector<Entity*>* collidableEntities,
	std::vector<Entity*>* collidableTiles,
	Grid* grid)
{
	_HandleCurrencies();
	_HanldeStageEnd();
	_HandleBonusItems();

	//----------------------------------------------------------------------------
	//TIMERS
	//----------------------------------------------------------------------------
	if (_health != 4 || (IsFlying() && GetTickCount64() - _flyStart > _flyTime)) {
		_flyStart = 0;
	}

	if (IsInPipe() && GetTickCount64() - _inPipeStart > _inPipeTime) {
		_inPipeStart = 0;
	}

	if (IsAttacking() && GetTickCount64() - _attackStart > _attackTime) {
		_attackStart = 0;
	}

	if (IsOnFireballCoolDown() && GetTickCount64() - _fireballCoolDownStart > _fireballCoolDownTime) {
		_fireballsCount = 0;
		_fireballCoolDownStart = 0;
	}

	if (IsInvulnerable() && GetTickCount64() - _invulnerableStart > _invulnerableTime) {
		_invulnerableStart = 0;

		_velocity = _originalVel;
	}
	//----------------------------------------------------------------------------
	//TIMERS
	//----------------------------------------------------------------------------

	//To show the whole kicking animation
	if (_isNextToShell && GetTickCount64() % 500 == 0) {
		_isNextToShell = false;
	}

	if (_health <= 0 || _isCrouching) {
		_velocity.x = 0.0f;
	}

	if (_triggeredStageEnd && !_hasBossItem) {
		MoveRight();
	}

	if (IsInPipe()) {
		_velocity.y = _health > 1 ? 0.035f : 0.03f;
		_velocity.y *= _normal.y;

		_isHolding = false;
		_velocity.x = 0.0f;

		if (GetTickCount64() - _inPipeStart == _inPipeTime * 0.5f) {
			_wentIntoPipe = !_wentIntoPipe;
			_position = _destination;
			_isOnGround = false;
			_normal.y *= _upVector;
		}
	}

	if (IsInvulnerable()) {
		if (GetTickCount64() - _invulnerableStart >= _invulnerableTime * 0.82f) {
			//Death bounce
			if (_health <= 0) {
				_isOnGround = false;
				_isHolding = false;

				_velocity.y = -_bounceSpeed;
				_gravity = 0.0010f;
			}
		}
		else {
			//Freeze the player
			_velocity = { 0.0f, 0.0f };
		}
	}

	_playerState->Update(deltaTime);
	Entity::Update(deltaTime, collidableEntities, collidableTiles, grid);

	if (_heldEntity != nullptr) {
		if (_heldEntity->GetHealth() == 0 || _heldEntity->GetHealth() == 3) {
			_isHolding = false;

			if (_heldEntity->GetHealth() == 3) {
				const float OFFSET_X = 17.0f;
				const float OFFSET_Y = 14.0f;
				_heldEntity->SetPosition(
					{
					_position.x + OFFSET_X * _normal.x,
					_position.y - OFFSET_Y
					}
				);
			}

			_heldEntity->isBeingHeld = false;
			_heldEntity = nullptr;
			return;
		}

		if (_isHolding) {
			D3DXVECTOR2 offset;
			offset.x = IsInPipe() ? 0.0f : 12.0f;
			offset.y = _health == 1 ? 2.0f : -3.0f;

			_heldEntity->SetPosition({ _position.x + offset.x * _normal.x, _position.y - offset.y });
		}
		else {
			_isNextToShell = true;

			_heldEntity->TakeDamage();
			_heldEntity->SetNormal({ -_normal.x, 0.0f });
			_heldEntity->isBeingHeld = false;
			_heldEntity = nullptr;
		}
	}
}

void Player::Render() {
	_playerState->Render();
}

void Player::Release() {
	if (_playerState != nullptr) {
		_playerState->Release();
		delete _playerState;
	}

	_animatedSprite.Release();
	_playerTexture = nullptr;
}