# Mundo 3D OpenGL - Mallas, Texturas y Grid Espacial

## Objetivo cumplido
Este proyecto integra un mundo 3D en OpenGL con:
- 5 mallas `.obj` del tema espacial.
- Un objeto texturizado con FreeImage.
- Estructura de clases (Mesh, Texture, SceneObject, Scene, SpatialGrid).
- Organizacion espacial con grid uniforme y debug visual.

## Assets usados (en `Mallas/`)
- `sun/sun.obj` (natural)
- `ImageToStl.com_asteroid_01/asteroid_01.obj` (natural)
- `barringer_meteorite_crater/barringer_meteorite_crater.obj` (natural)
- `Rocket Ship/10475_Rocket_Ship_v1_L3.obj` (artificial, objeto de la tribu, texturizado)
- `Space Station/Gateway Core.obj` (artificial)
- Textura: `Rocket Ship/10475_Rocket_Ship_v1_Diffuse.jpg`

## Controles
- `ESC`: cerrar ventana
- `s`: habilitar shaders
- `f`: usar fixed pipeline
- `g`: alternar debug de AABB (grid espacial)

## Compilacion (Visual Studio)
1. Abrir `TemplateProject.sln`.
2. Seleccionar `Debug | Win32`.
3. Compilar y ejecutar.

Si compilas en `Release`, asegurate de que `FreeImage.dll` quede junto al ejecutable en `Release/`.

## Compilacion por terminal (Windows)

### Usando Developer Command Prompt/PowerShell for VS
```bat
cd /d C:\Users\Usuario\Repositories\Computer-graphic-Model-Solaris-OpenGL
msbuild TemplateProject.sln /p:Configuration=Debug /p:Platform=Win32
copy FreeImage.dll Debug\
Debug\TemplateProject.exe
```

### Si `msbuild` no esta en PATH
Abre “Developer Command Prompt/PowerShell for VS” y repite los comandos anteriores.
Si aun falla, asegúrate de tener instalado **Desktop development with C++** en Visual Studio Installer.

## Evidencias
En `Evidencias/` estan las Fotos del resultado final, con los nombres:
- `Evidencias/Escena.jpeg`
- `Evidencias/Evidencia_OpenGL_Solaris.jpeg`

## Evidencia para la entrega
- Captura de pantalla con el mundo visible.
- Repositorio GitHub con commits de cada integrante.
