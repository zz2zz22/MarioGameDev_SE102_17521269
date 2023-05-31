#include "Device.h"
#include "PlayerState.h"
#include "IdleState.h"
#include "FallState.h"
#include "WagState.h"

FallState::FallState(Player* player) : PlayerState(player) {}

PlayerState* FallState::HandleStates() {
	if (_player->_isOnGround) {
		return new IdleState(_player);
	}
	//Attack in Raccoon form when falling
	else if (_player->IsAttacking()) {
		return new WagState(_player);
	}

	return nullptr;
}

void FallState::Render() {
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
		else if (_player->_acceleration >= _player->_ACCEL_THRESHOLD && _player->_heldEntity == nullptr) {
			_player->_animatedSprite.PlaySpriteAnimation("SuperJump", _player->_position, _player->_scale, _alpha);
		}
		else if (_player->_isNextToShell) {
			_player->_animatedSprite.PlaySpriteAnimation("Kick", _player->_position, _player->_scale, _alpha);
		}
		else if (_player->_heldEntity != nullptr) {
			_player->_animatedSprite.PlaySpriteAnimation("HoldJump", _player->_position, _player->_scale, _alpha);
		}
		else {
			_player->_animatedSprite.PlaySpriteAnimation("Jump", _player->_position, _player->_scale, _alpha);
		}
		break;
	case _Form::BIG:
		if (_player->IsInPipe()) {
			_player->_animatedSprite.PlaySpriteAnimation("BigFront", _player->_position);
		}
		else if (_player->_acceleration >= _player->_ACCEL_THRESHOLD && _player->_heldEntity == nullptr) {
			_player->_animatedSprite.PlaySpriteAnimation("BigSuperFall", _player->_position, _player->_scale, _alpha);
		}
		else if (_player->_isNextToShell) {
			_player->_animatedSprite.PlaySpriteAnimation("BigKick", _player->_position, _player->_scale, _alpha);
		}
		else if (_player->_heldEntity != nullptr) {
			_player->_animatedSprite.PlaySpriteAnimation("BigHoldJump", _player->_position, _player->_scale, _alpha);
		}
		else {
			_player->_animatedSprite.PlaySpriteAnimation("BigFall", _player->_position, _player->_scale, _alpha);
		}
		break;
	}
}