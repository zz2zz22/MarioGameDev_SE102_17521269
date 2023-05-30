#include "Device.h"
#include "PlayerState.h"
#include "IdleState.h"
#include "RunState.h"
#include "JumpState.h"
#include "FallState.h"
#include "CrouchState.h"

RunState::RunState(Player* player) : PlayerState(player) {}

PlayerState* RunState::HandleStates() {
	if (_player->_velocity.x == 0.0f) {
		return new IdleState(_player);
	}
	else if (_player->_velocity.y < 0.0f) {
		return new JumpState(_player);
	}
	else if (_player->_velocity.y > 0.0f && !_player->_isOnGround) {
		return new FallState(_player);
	}
	else if (_player->_isCrouching) {
		return new CrouchState(_player);
	}

	return nullptr;
}

void RunState::Render() {
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
		else if (_player->_acceleration >= _player->_ACCEL_THRESHOLD && _player->_isOnGround && _player->_heldEntity == nullptr) {
			_player->_animatedSprite.PlaySpriteAnimation("SuperRun", _player->_position, _player->_scale, _alpha);
		}
		else if (_player->_isNextToShell) {
			_player->_animatedSprite.PlaySpriteAnimation("Kick", _player->_position, _player->_scale, _alpha);
		}
		else if (_player->_heldEntity != nullptr) {
			_player->_animatedSprite.PlaySpriteAnimation("HoldRun", _player->_position, _player->_scale, _alpha);
		}
		else {
			if (_player->_acceleration < 0.5f && (Device::IsKeyDown(DIK_LEFTARROW) || Device::IsKeyDown(DIK_RIGHTARROW))) {
				_player->_animatedSprite.PlaySpriteAnimation("Skid", _player->_position, _player->_scale, _alpha);
			}
			else {
				_player->_animatedSprite.PlaySpriteAnimation("Run", _player->_position, _player->_scale, _alpha);
			}
		}
		break;
	}
}