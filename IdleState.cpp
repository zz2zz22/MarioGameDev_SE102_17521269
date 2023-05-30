#include "Device.h"
#include "PlayerState.h"
#include "IdleState.h"
#include "MapState.h"
#include "RunState.h"
#include "JumpState.h"
#include "FallState.h"
#include "CrouchState.h"
#include "ThrowState.h"
#include "WagState.h"

IdleState::IdleState(Player* player) : PlayerState(player) {}

PlayerState* IdleState::HandleStates() {
	if (_player->isInMap) {
		return new MapState(_player);
	}
	else if (_player->_velocity.x != 0.0f && _player->_isOnGround) {
		return new RunState(_player);
	}
	else if (_player->_velocity.y < 0.0f) {
		return new JumpState(_player);
	}
	else if (_player->_velocity.y > 0.0f && !_player->_isOnGround) {
		return new FallState(_player);
	}
	else if (_player->_heldEntity == nullptr) {
		if (_player->_isCrouching) {
			return new CrouchState(_player);
		}
		else if (Device::IsKeyDown(DIK_S) && _form == _Form::FIRE) {
			return new ThrowState(_player);
		}
		else if (_player->IsAttacking() && _form == _Form::RACCOON) {
			return new WagState(_player);
		}
	}
	return nullptr;
}

void IdleState::Render() {
	PlayerState::Render();
	if (_player->IsInvulnerable()) {
		return;
	}

	const float RAC_OFFSET = 4.0f;
	switch (_form) {
	case _Form::SMALL:
		if (_player->IsInPipe()) {
			_player->_animatedSprite.PlaySpriteAnimation("Front", _player->_position);
		}
		else if (_player->_isNextToShell) {
			_player->_animatedSprite.PlaySpriteAnimation("Kick", _player->_position, _player->_scale, _alpha);
		}
		else if (_player->_heldEntity != nullptr) {
			_player->_animatedSprite.PlaySpriteAnimation("HoldIdle", _player->_position, _player->_scale, _alpha);
		}
		else {
			_player->_animatedSprite.PlaySpriteAnimation("Idle", _player->_position, _player->_scale, _alpha);
		}
		break;
	}
}