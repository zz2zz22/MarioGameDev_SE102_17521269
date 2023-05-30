#include "Device.h"
#include "PlayerState.h"
#include "IdleState.h"
#include "CrouchState.h"

CrouchState::CrouchState(Player* player) : PlayerState(player) {}

PlayerState* CrouchState::HandleStates() {
	if (!_player->_isCrouching) {
		return new IdleState(_player);
	}

	return nullptr;
}

void CrouchState::Render() {
	PlayerState::Render();
	if (_player->IsInvulnerable()) {
		return;
	}

	const float RAC_OFFSET = 4.0f;
	const float CROUCH_OFFSET = 8.0f;
	switch (_form) {
	case _Form::SMALL:
		if (_player->IsInPipe()) {
			_player->_animatedSprite.PlaySpriteAnimation("Front", _player->_position);
		}
		else {
			_player->_animatedSprite.PlaySpriteAnimation("Idle", _player->_position, _player->_scale, _alpha);
		}
		break;
	}
}