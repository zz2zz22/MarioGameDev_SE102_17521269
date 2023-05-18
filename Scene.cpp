#include "GameUtils.h"
#include "Game.h"
#include "Scene.h"
#include "EntityList.h"

bool Scene::_IsEntityInViewport(Entity* entity, RECTF viewport) const {
	float entityWidth = entity->GetPosition().x + entity->GetBoxWidth();
	float entityHeight = entity->GetPosition().y + entity->GetBoxHeight();
	if (entityWidth >= viewport.left &&
		entityHeight >= viewport.top &&
		entity->GetPosition().x <= viewport.right &&
		entity->GetPosition().y <= viewport.bottom)
	{
		return true;
	}

	return false;
}

bool Scene::_IsEntityAliveAndIB(Entity* entity) const {
	float entityWidth = entity->GetPosition().x + entity->GetBoxWidth();
	float entityHeight = entity->GetPosition().y + entity->GetBoxHeight();
	if (entity->GetHealth() > -1 &&
		(entityWidth >= 0 &&
			entityHeight >= 0 &&
			entity->GetPosition().x <= _sceneWidth &&
			entity->GetPosition().y <= _sceneHeight))
	{
		return true;
	}

	entity->flaggedForRemoval = true;
	return false;
}

unsigned int Scene::_GetNextThemeID() {
	auto it = std::find(_mainThemeIDs.begin(), _mainThemeIDs.end(), _currentThemeID);
	++it;

	if (it >= _mainThemeIDs.end()) {
		it = _mainThemeIDs.begin();
	}

	_currentThemeID = *it;
	return _currentThemeID;
}

Texture* Scene::_LoadTexture(LPCWSTR filePath) {
	ID3D10Resource* resource = nullptr;
	ID3D10Texture2D* texture = nullptr;

	HRESULT hResult = D3DX10CreateTextureFromFile(
		GameUtils::directDevice,
		filePath,
		nullptr,
		nullptr,
		&resource,
		nullptr
	);
	if (FAILED(hResult)) {
		OutputDebugStringA("[SCENE] Failed to create texture from file\n");
		return nullptr;
	}

	//Converts the ID3D10Resource into a ID3D10Texture2D object
	resource->QueryInterface(__uuidof(ID3D10Texture2D), reinterpret_cast<LPVOID*>(&texture));
	resource->Release();

	if (texture == nullptr) {
		OutputDebugStringA("[SCENE] Failed to convert the resource into a texture2D\n");
		return nullptr;
	}

	D3D10_TEXTURE2D_DESC desc;
	texture->GetDesc(&desc);

	D3D10_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	ZeroMemory(&resourceViewDesc, sizeof(resourceViewDesc));
	resourceViewDesc.Format = desc.Format;
	resourceViewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MipLevels = desc.MipLevels;

	ID3D10ShaderResourceView* spriteTextureSRView = nullptr;
	GameUtils::directDevice->CreateShaderResourceView(texture, &resourceViewDesc, &spriteTextureSRView);

	return new Texture(texture, spriteTextureSRView);
}

void Scene::_ParseMainThemes(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 1) {
		return;
	}

	for (auto& token : tokens) {
		_mainThemeIDs.emplace_back(std::stoul(token));
	}
	_currentThemeID = _mainThemeIDs.front();
}

void Scene::_ParseSceneSize(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 2) {
		return;
	}

	_sceneWidth = std::stoul(tokens.at(0));
	_sceneHeight = std::stoul(tokens.at(1));
}

void Scene::_ParseSceneTime(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 1) {
		return;
	}

	_sceneTime = std::stoul(tokens.at(0));
}

void Scene::_ParseCameraLockValue(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.empty()) {
		return;
	}

	_lockValue = std::stof(tokens.at(0));
}

void Scene::_ParseCameraBounds(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 5) {
		return;
	}

	RECTF cameraBound;
	cameraBound.left = std::stof(tokens.at(0));
	cameraBound.top = std::stof(tokens.at(1));
	cameraBound.right = std::stof(tokens.at(2));
	cameraBound.bottom = std::stof(tokens.at(3));

	float upVector = std::stof(tokens.at(4));
	_cameraInstance->AddUpVector(upVector);

	_cameraInstance->AddCameraBound(cameraBound);
}

void Scene::_ParseBackgroundColor(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 3) {
		return;
	}

	float r = std::stof(tokens.at(0)) / 255.0f;
	float g = std::stof(tokens.at(1)) / 255.0f;
	float b = std::stof(tokens.at(2)) / 255.0f;
	_backgroundColor = { r, g, b, 1.0f };
}

void Scene::_ParseTextures(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 2) {
		return;
	}

	unsigned int textureID = std::stoul(tokens.at(0));

	Texture* texture = _LoadTexture(GameUtils::ToLPCWSTR(tokens.at(1)));
	_textureMap.insert(std::make_pair(textureID, texture));
}

void Scene::_ParseEntityData(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 5) {
		return;
	}

	std::vector<std::string> extraData;
	if (tokens.size() > 5) {
		for (unsigned int i = 5; i < tokens.size(); ++i) {
			extraData.emplace_back(tokens.at(i));
		}
	}

	GameObject::GameObjectType objectType = static_cast<GameObject::GameObjectType>(std::stoul(tokens.at(0)));

	float x = std::stof(tokens.at(3));
	float y = std::stof(tokens.at(4));
	D3DXVECTOR2 position = D3DXVECTOR2(x, y);

	unsigned int textureID = std::stoul(tokens.at(2));
	Texture* texture = GetTexture(textureID);

	Entity* entity = nullptr;
	//Set datafor object
	
	if (entity != nullptr) {
		entity->SetOjectType(objectType);
		entity->ParseData(tokens.at(1), texture, extraData);
		entity->SetPosition(position);

		_entities.emplace_back(entity);
	}
}

void Scene::_ParseTileData(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 5) {
		return;
	}

	GameObject::GameObjectType objectType = static_cast<GameObject::GameObjectType>(std::stoul(tokens.at(0)));

	float x = std::stof(tokens.at(1));
	float y = std::stof(tokens.at(2));
	D3DXVECTOR2 position = D3DXVECTOR2(x, y);

	RECTF hitbox;
	hitbox.left = -8.0f;
	hitbox.top = -8.0f;
	hitbox.right = std::stof(tokens.at(3));
	hitbox.bottom = std::stof(tokens.at(4));

	Tile* tile = new Tile;
	tile->SetOjectType(objectType);
	tile->SetPosition(position);
	tile->AddHitbox(hitbox);

	_tiles.emplace_back(tile);
}

void Scene::_ParseGrid(std::string line) {
	_grid = new Grid;
	_grid->ParseData(line, _entities);
}

void Scene::_ParseMainEffect(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 3) {
		return;
	}

	GameObject::GameObjectType objectType = static_cast<GameObject::GameObjectType>(std::stoul(tokens.at(0)));
	unsigned int textureID = std::stoul(tokens.at(2));
	Texture* texture = GetTexture(textureID);
}

void Scene::_ParseHUD(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() < 2) {
		return;
	}

	unsigned int textureID = std::stoul(tokens.at(1));
	Texture* texture = GetTexture(textureID);
}

void Scene::_ParseBackground(std::string line) {
	std::vector<std::string> tokens = GameUtils::SplitStr(line);

	if (tokens.size() == 1) {
		unsigned int textureID = std::stoul(tokens.at(0));
		Texture* texture = GetTexture(textureID);

		_background = new Background(texture);
		return;
	}

	if (tokens.size() < 6) {
		return;
	}

	RECT spriteBound;
	spriteBound.left = std::stoul(tokens.at(0));
	spriteBound.top = std::stoul(tokens.at(1));
	spriteBound.right = std::stoul(tokens.at(2));
	spriteBound.bottom = std::stoul(tokens.at(3));

	float x = std::stof(tokens.at(4));
	float y = std::stof(tokens.at(5));
	D3DXVECTOR2 position = D3DXVECTOR2(x, y);

	_background->AddSprite(spriteBound, position);
}

Scene::Scene(SceneType sceneID, std::string path) {
	_sceneID = sceneID;
	_filePath = path;
	_toSceneTime = 5000;
}

Scene::~Scene() {}

bool Scene::IsTransitioningToScene() const {
	return _toSceneStart != 0;
}

void Scene::StartToSceneTimer() {
	_toSceneStart = static_cast<DWORD>(GetTickCount64());
}

Scene::SceneType Scene::GetSceneID() const {
	return _sceneID;
}

unsigned int Scene::GetSceneWidth() const {
	return _sceneWidth;
}

unsigned int Scene::GetSceneHeight() const {
	return _sceneHeight;
}

D3DXCOLOR Scene::GetBGColor() const {
	return _backgroundColor;
}

Texture* Scene::GetTexture(unsigned int id) const {
	if (_textureMap.find(id) == _textureMap.end()) {
		char debug[100];
		sprintf_s(debug, "[SCENE] No valid texture with the corresponding ID: %lu\n", id);
		OutputDebugStringA(debug);
		return nullptr;
	}

	return _textureMap.at(id);
}

void Scene::HandleStates() {}

void Scene::OnKeyUp(int keyCode) {}

void Scene::OnKeyDown(int keyCode) {}

void Scene::AddEntityToScene(Entity* entity) {
	_entities.emplace_back(entity);
	if (_grid != nullptr) {
		_grid->AddEntity(entity);
	}
}

void Scene::RemoveEntityFromScene(Entity* entity) {
	if (_grid != nullptr) {
		_grid->RemoveEntity(entity);
	}
	_entities.erase(std::remove(_entities.begin(), _entities.end(), entity), _entities.end());
}

Entity* Scene::CreateEntityFromData(std::string objectID, std::string dataPath, std::string textureID) {
	Entity* entity = nullptr;

	GameObject::GameObjectType objectType = static_cast<GameObject::GameObjectType>(std::stoul(objectID));
	unsigned int texID = std::stoul(textureID);
	Texture* texture = GetTexture(texID);

	entity->SetOjectType(objectType);
	entity->ParseData(dataPath, texture);
	return entity;
}

void Scene::LoadScene() {
	char debug[100];
	sprintf_s(debug, "[SCENE] Loading scene with ID: %d\n", _sceneID);
	OutputDebugStringA(debug);

	std::ifstream readFile;
	readFile.open(_filePath, std::ios::in);

	if (!readFile.is_open()) {
		OutputDebugStringA("[SCENE] Failed to read file\n");
		return;
	}

	//Load objects here, cause the Scene won't be calling destructor before the game ends	
	const unsigned int MAX_ENTITIES_PER_SCENE = 256;
	_entities.reserve(MAX_ENTITIES_PER_SCENE);
	_tiles.reserve(MAX_ENTITIES_PER_SCENE);

	_background = nullptr;
	_grid = nullptr;
	_cameraInstance = Camera::GetInstance();

	_SceneFileSection sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_UNKNOWN;

	char str[GameUtils::MAX_FILE_LINE];
	while (readFile.getline(str, GameUtils::MAX_FILE_LINE)) {
		std::string line(str);

		if (line.empty() || line.front() == '#') {
			continue;
		}

		if (line == "[/]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_UNKNOWN;
			continue;
		}

		if (line == "[MAINTHEMES]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_MAINTHEMES;
			continue;
		}

		if (line == "[SCENESIZE]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_SCENESIZE;
			continue;
		}

		if (line == "[SCENETIME]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_SCENETIME;
			continue;
		}

		if (line == "[CAMERALOCKVALUE]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_CAMERALOCKVALUE;
			continue;
		}

		if (line == "[CAMERABOUNDS]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_CAMERABOUNDS;
			continue;
		}

		if (line == "[BGCOLOR]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_BGCOLOR;
			continue;
		}

		if (line == "[TEXTURES]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_TEXTURES;
			continue;
		}

		if (line == "[ENTITYDATA]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_ENTITYDATA;
			continue;
		}

		if (line == "[TILEDATA]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_TILEDATA;
			continue;
		}

		if (line == "[GRID]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_GRID;
			continue;
		}

		if (line == "[HUD]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_HUD;
			continue;
		}

		if (line == "[MAINEFFECT]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_MAINEFFECT;
			continue;
		}

		if (line == "[BACKGROUND]") {
			sceneFileSection = _SceneFileSection::SCENEFILE_SECTION_BACKGROUND;
			continue;
		}

		switch (sceneFileSection) {
		case _SceneFileSection::SCENEFILE_SECTION_MAINTHEMES:
			_ParseMainThemes(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_SCENESIZE:
			_ParseSceneSize(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_SCENETIME:
			_ParseSceneTime(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_CAMERALOCKVALUE:
			_ParseCameraLockValue(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_CAMERABOUNDS:
			_ParseCameraBounds(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_BGCOLOR:
			_ParseBackgroundColor(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_TEXTURES:
			_ParseTextures(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_ENTITYDATA:
			_ParseEntityData(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_TILEDATA:
			_ParseTileData(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_GRID:
			_ParseGrid(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_HUD:
			_ParseHUD(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_MAINEFFECT:
			_ParseMainEffect(line);
			break;
		case _SceneFileSection::SCENEFILE_SECTION_BACKGROUND:
			_ParseBackground(line);
			break;
		}
	}

	readFile.close();

	sprintf_s(debug, "[SCENE] Loaded scene with ID: %d\n", _sceneID);
	OutputDebugStringA(debug);
}

void Scene::UpdateCameraPosition() {}

void Scene::Update(DWORD deltaTime) {}

void Scene::Render() {}