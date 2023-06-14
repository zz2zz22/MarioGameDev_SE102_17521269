<p align="center">
  <h1 align="center">
    --SCHOOL PROJECT--
    GAME DEVELOP - SUPER MARIO BROS 3
  </h1>
</p>

🎓 A solo school project on how to create a 2D platformer game.

`game-development` `C++` `school-project` `DirectX 10` `OOP`

---

## 🏃 Project request ##
- World 1-1 & World map & Static Intro Scene

## Requirements
+ OS: Windows® 10 32-bit+
+ Processor: Any modern processor
+ Memory: 512 MB+
+ Graphics: Intergrated graphics
+ DirectX: Version 10


### 1️⃣ How to run ###
Only do this if the project isn't running.
+ Download VS2019 + [here](https://visualstudio.microsoft.com/vs/)
+ Linker > Input > Additional Dependencies:\
`d3d10.lib;d3dcompiler.lib;dxguid.lib;dinput8.lib;`
+ Tools > NuGet Package Manager > Package Manager Console and paste this in\
`Install-Package Microsoft.DXSDK.D3DX`
+ [***IMPORTANT***] Configuration: `Active(Debug)`, Platform: `Win32`

### 2️⃣ Controls ###
|Constraint |Key            |Action                  |
|-----------|---------------|------------------------|
|Global     |<kbd>Esc</kbd> |Exit application        |
|Stage only |<kbd>⬆</kbd>   |Enter pipe/door         |
|           |<kbd>⬅</kbd>  |Move left               |
|           |<kbd>⬇</kbd>   |Crouch/Enter pipe       |
|           |<kbd>➡</kbd>  |Move right              |
|           |<kbd>S</kbd>   |Throw/Attack/Hold shell |
|           |<kbd>␣</kbd>   |Run/Fly/Jump/Float down |
|Debug only |<kbd>1</kbd>   |Set form to `SMALL`     |
|           |<kbd>2</kbd>   |Set form to `BIG`       |
|           |<kbd>3</kbd>   |Set form to `FIRE`      |
|           |<kbd>4</kbd>   |Set form to `RACCOON`   |
|           |<kbd>R</kbd>   |Reset scene             |

## Credits
+ [The Spriter Resources](https://www.spriters-resource.com/nes/supermariobros3/)
+ [nesmaps](https://nesmaps.com/maps/SuperMarioBrothers3/SuperMarioBrothers3.html)
+ [Game logic research](https://www.mariowiki.com)
+ [Framework tutorial](https://github.com/dungdna2000/gamedev-intro-tutorials)
