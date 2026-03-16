# Mundo 3D OpenGL - Mallas, Texturas y Grid Espacial

## Objetivo
Proyecto con mundo 3D en OpenGL que integra:
- 5 mallas `.obj` espaciales.
- Un objeto texturizado con FreeImage.
- Estructura de clases clara (Mesh, Texture, SceneObject, Scene, SpatialGrid).
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

## Evidencias
Guarda tus capturas en `Evidencias/` y actualiza esta seccion.

Ejemplos de nombres sugeridos:
- `Evidencias/Escena.jpeg`
- `Evidencias/Evidencia_OpenGL_Solaris.jpeg`

