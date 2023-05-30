#include "Device.h"
#include "PlayerState.h"
#include "JumpState.h"
#include "FallState.h"

JumpState::JumpState(Player* player) : PlayerState(player) {}

PlayerState* JumpState::HandleStates() {
	if (_player->_velocity.y > 0.0f) {
		return new FallState(_player);
	}
	return nullptr;
}

void JumpState::Render() {
	PlayerState::Render();
	if (_player->IsInvulnerable()) {
		return;
	}

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
	}
}