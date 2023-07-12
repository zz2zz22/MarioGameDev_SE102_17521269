#include "SceneManager.h"
#include "Scene.h"
#include "SceneIntro.h"
#include "EntityList.h"

SceneIntro::SceneIntro(SceneType sceneID, std::string path) : Scene(sceneID, path) {
	_toSceneTime = 1000;
}

SceneIntro::~SceneIntro() {}

void SceneIntro::OnKeyDown(int keyCode) {
	switch (keyCode) {
	case DIK_U:
		_selectText->isMultiplayer = !_selectText->isMultiplayer;
		break;
	case DIK_I:
		if (!IsTransitioningToScene()) {
			StartToSceneTimer();
		}
		break;
	}
}

void SceneIntro::LoadScene() {
	Scene::LoadScene();
	_introTimeStart = static_cast<DWORD>(GetTickCount64());
}

void SceneIntro::Update(DWORD deltaTime) {
	const DWORD INTRO_TIME = 60000;
	DWORD time = static_cast<DWORD>(GetTickCount64()) - _introTimeStart;
	if (time > INTRO_TIME) {
		time = INTRO_TIME;
	}
	const D3DXCOLOR BG_COLOR = { 252 / 255.0f, 216 / 255.0f, 168 / 255.0f, 1.0f };
	_backgroundColor = BG_COLOR;

	for (auto it = _entities.begin(); it != _entities.end(); ++it) {
		Entity* entity = *it;
		if (entity->GetObjectType() == GameObject::GameObjectType::GAMEOBJECT_TYPE_SELECT) {
			continue;
		}
		entity->SetScale({ 1.0f, 1.0f });
	}

	for (auto it = _entities.begin(); it != _entities.end(); ++it) {
		Entity* entity = *it;
		switch (entity->GetObjectType()) {
		case GameObject::GameObjectType::GAMEOBJECT_TYPE_LOGO:
		{
			GameLogo* gameLogo = dynamic_cast<GameLogo*>(entity);
			if (time >= 0 && time < 200) {
				gameLogo->StartFallDownTimer();
			}
		}
		break;
		case GameObject::GameObjectType::GAMEOBJECT_TYPE_SELECT:
			_selectText->SetScale({ 1.0f, 1.0f });
			break;
		}

		entity->Update(deltaTime, &_entities, &_tiles);
	}
	std::sort(_entities.begin(), _entities.end(), Entity::CompareRenderPriority);

	if (IsTransitioningToScene() && GetTickCount64() - _toSceneStart > _toSceneTime) {
		_toSceneStart = 0;
		SceneManager::GetInstance()->ChangeScene(static_cast<unsigned int>(SceneType::SCENE_TYPE_MAP));
	}
}

void SceneIntro::Render() {
	_background->BruteForceRender();

	for (auto it = _entities.begin(); it != _entities.end(); ++it) {
		(*it)->Render();
	}
}

void SceneIntro::Release() {
	char debug[100];
	sprintf_s(debug, "[SCENE] Unloading scene with ID: %d\n", _sceneID);
	OutputDebugStringA(debug);

	if (_background != nullptr) {
		_background->Release();
		delete _background;
	}

	for (auto& tile : _tiles) {
		tile->Release();
		delete tile;
	}
	_tiles.clear();

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