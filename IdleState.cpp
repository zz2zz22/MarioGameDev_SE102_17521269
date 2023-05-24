#include "Device.h"
#include "PlayerState.h"
#include "IdleState.h"

IdleState::IdleState(Player* player) : PlayerState(player) {}

PlayerState* IdleState::HandleStates() {
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