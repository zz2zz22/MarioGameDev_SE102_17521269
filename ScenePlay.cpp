#include <random>

#include "Game.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ScenePlay.h"
#include "EntityList.h"

ScenePlay::ScenePlay(SceneType sceneID, std::string path) : Scene(sceneID, path) {
	_hurryUpTime = 3000;
	_pitch = 1.0f;
}

ScenePlay::~ScenePlay() {}

bool ScenePlay::IsInHurry() const {
	return _hurryUpStart != 0;
}

void ScenePlay::StartHurryingUpTimer() {
	_hurryUpStart = static_cast<DWORD>(GetTickCount64());
}

void ScenePlay::HandleStates() {
	_player->HandleStates();
}

void ScenePlay::OnKeyUp(int keyCode) {
	_player->OnKeyUpGame(keyCode);
}

void ScenePlay::OnKeyDown(int keyCode) {
	switch (keyCode) {
	case DIK_1:
		_player->SetHealth(1);
		break;
	case DIK_2:
		if (_player->GetHealth() == 1) {
			_player->SetPosition({ _player->GetPosition().x, _player->GetPosition().y - _player->GetBoxHeight(1) });
		}
		_player->SetHealth(2);
		break;
	case DIK_3:
		if (_player->GetHealth() == 1) {
			_player->SetPosition({ _player->GetPosition().x, _player->GetPosition().y - _player->GetBoxHeight(1) });
		}
		_player->SetHealth(3);
		break;
	case DIK_4:
		if (_player->GetHealth() == 1) {
			_player->SetPosition({ _player->GetPosition().x, _player->GetPosition().y - _player->GetBoxHeight(1) });
		}
		_player->SetHealth(4);
		break;
	}

	_player->OnKeyDownGame(keyCode);
}

void ScenePlay::LoadScene() {
	Scene::LoadScene();
}

void ScenePlay::UpdateCameraPosition() {
	unsigned int index = _player->WentIntoPipe();
	RECTF cameraBound = _cameraInstance->GetCameraBound(index);
	_player->SetUpVector(_cameraInstance->GetUpVector(index));
	D3DXVECTOR2 cameraPosition = _cameraInstance->GetPosition();

	if (!_player->TriggeredStageEnd() && !_player->IsInPipe()) {
		if (_player->GetPosition().x < cameraPosition.x) {
			_player->SetPosition({ cameraPosition.x, _player->GetPosition().y });
		}
		else if (_player->GetPosition().x + _player->GetBoxWidth() > _sceneWidth) {
			_player->SetPosition({ _sceneWidth - _player->GetBoxWidth(), _player->GetPosition().y });
		}

		if (_player->GetPosition().y < cameraBound.top) {
			_player->SetPosition({ _player->GetPosition().x, cameraPosition.y });
		}
	}

	cameraPosition = _player->GetPosition();
	if (!_player->lockCameraXAxis) {
		cameraPosition.x -= Game::GetInstance()->GetWindowWidth() / 2.25f;
	}
	if (cameraPosition.x < cameraBound.left) {
		cameraPosition.x = cameraBound.left;
	}
	else if (cameraPosition.x + Game::GetInstance()->GetWindowWidth() > cameraBound.right) {
		cameraPosition.x = cameraBound.right - Game::GetInstance()->GetWindowWidth();
	}

	cameraPosition.y -= Game::GetInstance()->GetWindowHeight() / 2.25f;
	if (_player->WentIntoPipe() || _player->IsFlying() || _player->GetPosition().y < _sceneHeight * _lockValue) {
		if (cameraPosition.y < cameraBound.top) {
			cameraPosition.y = cameraBound.top;
		}
		else if (cameraPosition.y + Game::GetInstance()->GetWindowHeight() > cameraBound.bottom) {
			cameraPosition.y = cameraBound.bottom - Game::GetInstance()->GetWindowHeight();
		}
	}
	else {
		cameraPosition.y = cameraBound.bottom - Game::GetInstance()->GetWindowHeight();
	}

	_cameraInstance->SetPosition(cameraPosition);
}

void ScenePlay::Update(DWORD deltaTime) {
	if (_player == nullptr) {
		char debug[100];
		sprintf_s(debug, "[SCENE] No player loaded in, scene ID: %d\n", _sceneID);
		OutputDebugStringA(debug);
		return;
	}

	if (_player->GetHealth() > 0 && !_player->TriggeredStageEnd()) {
		if (_sceneTime > 0 && GetTickCount64() % 1000 == 0) {
			--_sceneTime;
		}
	}

	if ((_sceneTime == 0 && !_player->TriggeredStageEnd() && _player->GetHealth() != 0) ||
		_player->GetPosition().y > _sceneHeight)
	{
		if (_player->GetHealth() != 0) {
			_player->SetHealth(1);
			_player->TakeDamage();
		}
	}
	if (_sceneTime == _NEAR_TIME_LIMIT && !_isInHurry) {
		_isInHurry = true;
		_hurryUp = true;

		StartHurryingUpTimer();
	}

	if (_isInHurry && _hurryUp) {
		_hurryUp = false;
		_pitch = 1.04f;
	}

	if (IsInHurry() && GetTickCount64() - _hurryUpStart > _hurryUpTime) {
		_hurryUpStart = 0;
	}

	if (_player->IsInPipe()) {
		if (_player->WentIntoPipe() && !_isInSecret) {
			_isInSecret = true;
		}
		else if (!_player->WentIntoPipe() && _isInSecret) {
			_isInSecret = false;
		}
	}
	if (_player->GetHealth() == 0 || _player->IsInvulnerable()) {
		_player->Update(deltaTime, &_entities, &_tiles, _grid);
	}
	else if (_player->GetHealth() > 0 && !_player->IsInvulnerable()) {
		//Range-based loop, for_each, iterators will all be invalidated if an element is either removed or inserted
		//And the container has to do a reallocation
		for (unsigned int i = 0; i < _entities.size(); ++i) {
			Entity* entity = _entities.at(i);
			entity->SetActive(_IsEntityInViewport(entity, _cameraInstance->GetViewport()));
			entity->Update(deltaTime, &_entities, &_tiles, _grid);

			//Entities events

			if (_grid != nullptr) {
				Cell* newCell = _grid->GetCell(entity->GetPosition());
				if (newCell != entity->ownerCell) {
					_grid->RemoveEntity(entity);
					_grid->AddEntity(entity, newCell);
				}
			}

			if (!_IsEntityAliveAndIB(entity)) {
				if (_grid != nullptr) {
					_grid->RemoveEntity(entity);
				}

				_removedEntities.emplace_back(entity);

				_entities.erase(std::remove(_entities.begin(), _entities.end(), entity), _entities.end());
			}
		}
		std::sort(_entities.begin(), _entities.end(), Entity::CompareRenderPriority);
	}

	UpdateCameraPosition();

	const float HUD_OFFSET_X = 134.0f;
	const float HUD_OFFSET_Y = 161.0f;

	_background->Update();

	if (_player->TriggeredStageEnd() || _player->GetHealth() == 0 || _sceneTime == 0) {
		//Warp back to map				
		if (!IsTransitioningToScene()) {
			StartToSceneTimer();

			_player->GetSceneRemainingTime(_sceneTime);

			_sceneTime = 0;
		}
	}
}

void ScenePlay::Render() {
	_background->Render();

	for (unsigned int i = 0; i < _entities.size(); ++i) {
		Entity* entity = _entities.at(i);
		if (entity->IsActive()) {
			entity->Render();
		}
	}
}

void ScenePlay::Release() {
	char debug[100];
	sprintf_s(debug, "[SCENE] Unloading scene with ID: %d\n", _sceneID);
	OutputDebugStringA(debug);

	_background->Release();
	delete _background;

	if (_grid != nullptr) {
		_grid->Release();
		delete _grid;
	}

	for (auto& tile : _tiles) {
		tile->Release();
		delete tile;
	}
	_tiles.clear();

	for (unsigned int i = 0; i < _removedEntities.size(); ++i) {
		_removedEntities.at(i)->Release();
		delete _removedEntities.at(i);
	}
	_removedEntities.clear();

	for (unsigned int i = 0; i < _entities.size(); ++i) {
		_entities.at(i)->Release();
		delete _entities.at(i);
	}
	_entities.clear();

	for (auto& texture : _textureMap) {
		texture.second->Release();
		delete texture.second;
	}
	_textureMap.clear();

	if (_cameraInstance != nullptr) {
		_cameraInstance->Release();
	}

	sprintf_s(debug, "[SCENE] Unloaded scene with ID: %d\n", _sceneID);
	OutputDebugStringA(debug);
}