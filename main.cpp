//=============================================================================
// Sample Application: Lighting (Per Fragment Phong)
//=============================================================================

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "glApplication.h"
#include "glutWindow.h"
#include <iostream>
#include "glsl.h"
#include <time.h>
#include "glm.h"
#include <FreeImage.h>

#include <vector>
#include <string>
#include <map>
#include <set>
#include <cmath>
#include <memory>
#include <cstdlib>

//-----------------------------------------------------------------------------

struct Vec3
{
   float x;
   float y;
   float z;

   Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
   Vec3(float xVal, float yVal, float zVal) : x(xVal), y(yVal), z(zVal) {}

   Vec3 operator+(const Vec3& other) const
   {
      return Vec3(x + other.x, y + other.y, z + other.z);
   }

   Vec3 operator-(const Vec3& other) const
   {
      return Vec3(x - other.x, y - other.y, z - other.z);
   }

   Vec3 operator*(float s) const
   {
      return Vec3(x * s, y * s, z * s);
   }
};

static Vec3 Mul(const Vec3& a, const Vec3& b)
{
   return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

struct AABB
{
   Vec3 min;
   Vec3 max;
};

//-----------------------------------------------------------------------------

class Mesh
{
public:
   explicit Mesh(const std::string& path)
      : model(nullptr), dimensions(1.0f, 1.0f, 1.0f)
   {
      model = glmReadOBJ(const_cast<char*>(path.c_str()));
      if (!model)
      {
         std::cerr << "Failed to load OBJ: " << path << "\n";
         std::exit(1);
      }

      glmUnitize(model);
      glmFacetNormals(model);
      glmVertexNormals(model, 90.0f);

      GLfloat dims[3];
      glmDimensions(model, dims);
      dimensions = Vec3(dims[0], dims[1], dims[2]);
   }

   ~Mesh()
   {
      if (model)
      {
         glmDelete(model);
         model = nullptr;
      }
   }

   GLMmodel* get() const { return model; }
   Vec3 getDimensions() const { return dimensions; }

private:
   GLMmodel* model;
   Vec3 dimensions;
};

//-----------------------------------------------------------------------------

class Texture
{
public:
   Texture() : id(0), width(0), height(0) {}

   ~Texture()
   {
      if (id != 0)
      {
         glDeleteTextures(1, &id);
         id = 0;
      }
   }

   bool load(const std::string& path)
   {
      FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.c_str(), 0);
      if (fif == FIF_UNKNOWN)
      {
         fif = FreeImage_GetFIFFromFilename(path.c_str());
      }
      if (fif == FIF_UNKNOWN)
      {
         std::cerr << "Unknown image format: " << path << "\n";
         return false;
      }

      FIBITMAP* bitmap = FreeImage_Load(fif, path.c_str());
      if (!bitmap)
      {
         std::cerr << "Failed to load image: " << path << "\n";
         return false;
      }

      FIBITMAP* pImage = FreeImage_ConvertTo32Bits(bitmap);
      FreeImage_Unload(bitmap);
      if (!pImage)
      {
         std::cerr << "Failed to convert image: " << path << "\n";
         return false;
      }

      width = FreeImage_GetWidth(pImage);
      height = FreeImage_GetHeight(pImage);

      if (id != 0)
      {
         glDeleteTextures(1, &id);
         id = 0;
      }

      glGenTextures(1, &id);
      glBindTexture(GL_TEXTURE_2D, id);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA,
                   GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pImage));

      glBindTexture(GL_TEXTURE_2D, 0);
      FreeImage_Unload(pImage);
      return true;
   }

   GLuint getId() const { return id; }

private:
   GLuint id;
   int width;
   int height;
};

//-----------------------------------------------------------------------------

class SceneObject
{
public:
   explicit SceneObject(Mesh* meshPtr)
      : mesh(meshPtr),
        texture(nullptr),
        position(0.0f, 0.0f, 0.0f),
        rotation(0.0f, 0.0f, 0.0f),
        scale(1.0f, 1.0f, 1.0f)
   {}

   void setTexture(Texture* tex)
   {
      texture = tex;
   }

   bool hasTexture() const
   {
      return texture != nullptr;
   }

   AABB getAABB() const
   {
      Vec3 half = Mul(mesh->getDimensions(), Vec3(0.5f, 0.5f, 0.5f));
      half = Mul(half, scale);
      AABB box;
      box.min = position - half;
      box.max = position + half;
      return box;
   }

   void draw(bool useTexture) const
   {
      glPushMatrix();
      glTranslatef(position.x, position.y, position.z);
      glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
      glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
      glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);
      glScalef(scale.x, scale.y, scale.z);

      if (useTexture && texture)
      {
         glEnable(GL_TEXTURE_2D);
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, texture->getId());
         glmDraw(mesh->get(), GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
      }
      else
      {
         glBindTexture(GL_TEXTURE_2D, 0);
         glDisable(GL_TEXTURE_2D);
         glmDraw(mesh->get(), GLM_SMOOTH | GLM_MATERIAL);
      }

      glPopMatrix();
   }

   Vec3 position;
   Vec3 rotation;
   Vec3 scale;

private:
   Mesh* mesh;
   Texture* texture;
};

//-----------------------------------------------------------------------------

struct CellKey
{
   int x;
   int y;
   int z;
};

struct CellKeyLess
{
   bool operator()(const CellKey& a, const CellKey& b) const
   {
      if (a.x != b.x) return a.x < b.x;
      if (a.y != b.y) return a.y < b.y;
      return a.z < b.z;
   }
};

class SpatialGrid
{
public:
   explicit SpatialGrid(float size) : cellSize(size) {}

   void clear()
   {
      cells.clear();
   }

   void build(const std::vector<SceneObject*>& objects)
   {
      clear();
      for (SceneObject* obj : objects)
      {
         insert(obj);
      }
   }

   const std::map<CellKey, std::vector<SceneObject*>, CellKeyLess>& getCells() const
   {
      return cells;
   }

private:
   CellKey cellForPoint(const Vec3& p) const
   {
      CellKey key;
      key.x = static_cast<int>(std::floor(p.x / cellSize));
      key.y = static_cast<int>(std::floor(p.y / cellSize));
      key.z = static_cast<int>(std::floor(p.z / cellSize));
      return key;
   }

   void insert(SceneObject* obj)
   {
      AABB box = obj->getAABB();
      CellKey minKey = cellForPoint(box.min);
      CellKey maxKey = cellForPoint(box.max);

      for (int x = minKey.x; x <= maxKey.x; ++x)
      {
         for (int y = minKey.y; y <= maxKey.y; ++y)
         {
            for (int z = minKey.z; z <= maxKey.z; ++z)
            {
               CellKey key;
               key.x = x;
               key.y = y;
               key.z = z;
               cells[key].push_back(obj);
            }
         }
      }
   }

   float cellSize;
   std::map<CellKey, std::vector<SceneObject*>, CellKeyLess> cells;
};

//-----------------------------------------------------------------------------

static void DrawEdge(const Vec3& a, const Vec3& b)
{
   glVertex3f(a.x, a.y, a.z);
   glVertex3f(b.x, b.y, b.z);
}

static void DrawAABB(const AABB& box)
{
   Vec3 v000(box.min.x, box.min.y, box.min.z);
   Vec3 v001(box.min.x, box.min.y, box.max.z);
   Vec3 v010(box.min.x, box.max.y, box.min.z);
   Vec3 v011(box.min.x, box.max.y, box.max.z);
   Vec3 v100(box.max.x, box.min.y, box.min.z);
   Vec3 v101(box.max.x, box.min.y, box.max.z);
   Vec3 v110(box.max.x, box.max.y, box.min.z);
   Vec3 v111(box.max.x, box.max.y, box.max.z);

   glBegin(GL_LINES);
   DrawEdge(v000, v001);
   DrawEdge(v001, v011);
   DrawEdge(v011, v010);
   DrawEdge(v010, v000);

   DrawEdge(v100, v101);
   DrawEdge(v101, v111);
   DrawEdge(v111, v110);
   DrawEdge(v110, v100);

   DrawEdge(v000, v100);
   DrawEdge(v001, v101);
   DrawEdge(v010, v110);
   DrawEdge(v011, v111);
   glEnd();
}

//-----------------------------------------------------------------------------

class Scene
{
public:
   SceneObject* addObject(Mesh* mesh)
   {
      objects.push_back(std::unique_ptr<SceneObject>(new SceneObject(mesh)));
      return objects.back().get();
   }

   void buildSpatialGrid(SpatialGrid& grid) const
   {
      std::vector<SceneObject*> ptrs;
      ptrs.reserve(objects.size());
      for (const std::unique_ptr<SceneObject>& obj : objects)
      {
         ptrs.push_back(obj.get());
      }
      grid.build(ptrs);
   }

   void render(const SpatialGrid& grid,
               cwc::glShader* shaderPhong,
               cwc::glShader* shaderTex,
               bool debug) const
   {
      std::set<const SceneObject*> drawn;

      const std::map<CellKey, std::vector<SceneObject*>, CellKeyLess>& cells = grid.getCells();
      for (std::map<CellKey, std::vector<SceneObject*>, CellKeyLess>::const_iterator it = cells.begin();
           it != cells.end(); ++it)
      {
         const std::vector<SceneObject*>& cellObjects = it->second;
         for (size_t i = 0; i < cellObjects.size(); ++i)
         {
            SceneObject* obj = cellObjects[i];
            if (drawn.insert(obj).second)
            {
               if (obj->hasTexture())
               {
                  if (shaderTex) shaderTex->begin();
                  if (shaderTex) shaderTex->setUniform1i((GLcharARB*)"myTexture", 0);
                  obj->draw(true);
                  if (shaderTex) shaderTex->end();
               }
               else
               {
                  if (shaderPhong) shaderPhong->begin();
                  obj->draw(false);
                  if (shaderPhong) shaderPhong->end();
               }
            }
         }
      }

      if (debug)
      {
         glUseProgram(0);
         glDisable(GL_LIGHTING);
         glColor3f(1.0f, 1.0f, 0.0f);
         for (std::set<const SceneObject*>::const_iterator it = drawn.begin();
              it != drawn.end(); ++it)
         {
            DrawAABB((*it)->getAABB());
         }
         glEnable(GL_LIGHTING);
      }
   }

private:
   std::vector<std::unique_ptr<SceneObject> > objects;
};

//-----------------------------------------------------------------------------

class myWindow : public cwc::glutWindow
{
protected:
   cwc::glShaderManager SM;
   cwc::glShader* shaderPhong;
   cwc::glShader* shaderTex;
   clock_t time0, time1;
   float timer010;
   bool bUp;
   bool showGrid;

   Scene scene;
   SpatialGrid grid;
   std::vector<std::unique_ptr<Mesh> > meshes;
   std::vector<std::unique_ptr<Texture> > textures;

public:
   myWindow()
      : shaderPhong(nullptr),
        shaderTex(nullptr),
        time0(0),
        time1(0),
        timer010(0.0f),
        bUp(true),
        showGrid(false),
        grid(3.0f)
   {}

   virtual void OnRender(void)
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glPushMatrix();
      glRotatef(timer010 * 360.0f, 0.0f, 1.0f, 0.0f);

      scene.render(grid, shaderPhong, shaderTex, showGrid);

      glPopMatrix();
      glutSwapBuffers();

      UpdateTimer();
      Repaint();
   }

   virtual void OnIdle() {}

   virtual void OnInit()
   {
      FreeImage_Initialise(TRUE);

      glClearColor(0.5f, 0.5f, 1.0f, 0.0f);
      glShadeModel(GL_SMOOTH);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);

      shaderPhong = SM.loadfromFile((char*)"vertexshader.txt", (char*)"fragmentshader.txt");
      if (shaderPhong == 0)
         std::cout << "Error Loading, compiling or linking shader\n";

      shaderTex = SM.loadfromFile((char*)"vertexshaderT.txt", (char*)"fragmentshaderT.txt");
      if (shaderTex == 0)
         std::cout << "Error Loading, compiling or linking shader\n";

      time0 = clock();
      timer010 = 0.0f;
      bUp = true;

      // Load meshes (space assets)
      meshes.push_back(std::unique_ptr<Mesh>(new Mesh("./Mallas/sun/sun.obj")));
      meshes.push_back(std::unique_ptr<Mesh>(new Mesh("./Mallas/ImageToStl.com_asteroid_01/asteroid_01.obj")));
      meshes.push_back(std::unique_ptr<Mesh>(new Mesh("./Mallas/barringer_meteorite_crater/barringer_meteorite_crater.obj")));
      meshes.push_back(std::unique_ptr<Mesh>(new Mesh("./Mallas/Rocket Ship/10475_Rocket_Ship_v1_L3.obj")));
      meshes.push_back(std::unique_ptr<Mesh>(new Mesh("./Mallas/Space Station/Gateway Core.obj")));

      SceneObject* sun = scene.addObject(meshes[0].get());
      sun->position = Vec3(-2.5f, 1.0f, -3.0f);
      sun->scale = Vec3(1.0f, 1.0f, 1.0f);

      SceneObject* asteroid = scene.addObject(meshes[1].get());
      asteroid->position = Vec3(-1.0f, -0.8f, -2.2f);
      asteroid->scale = Vec3(0.7f, 0.7f, 0.7f);
      asteroid->rotation = Vec3(0.0f, 25.0f, 0.0f);

      SceneObject* crater = scene.addObject(meshes[2].get());
      crater->position = Vec3(2.2f, -1.0f, -3.5f);
      crater->scale = Vec3(1.1f, 1.1f, 1.1f);
      crater->rotation = Vec3(0.0f, -20.0f, 0.0f);

      SceneObject* rocket = scene.addObject(meshes[3].get());
      rocket->position = Vec3(1.2f, 0.8f, -2.5f);
      rocket->scale = Vec3(0.8f, 0.8f, 0.8f);
      rocket->rotation = Vec3(-10.0f, 30.0f, 0.0f);

      SceneObject* station = scene.addObject(meshes[4].get());
      station->position = Vec3(0.0f, 1.2f, -4.0f);
      station->scale = Vec3(0.6f, 0.6f, 0.6f);
      station->rotation = Vec3(0.0f, -45.0f, 0.0f);

      // Load texture for rocket (tribe object + textured)
      textures.push_back(std::unique_ptr<Texture>(new Texture()));
      if (textures.back()->load("./Mallas/Rocket Ship/10475_Rocket_Ship_v1_Diffuse.jpg"))
      {
         rocket->setTexture(textures.back().get());
      }

      // Build spatial grid once
      scene.buildSpatialGrid(grid);

      DemoLight();
   }

   virtual void OnResize(int w, int h)
   {
      if (h == 0) h = 1;
      float ratio = 1.0f * (float)w / (float)h;

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      glViewport(0, 0, w, h);

      gluPerspective(45, ratio, 1, 100);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      gluLookAt(0.0f, 0.0f, 4.0f,
                0.0f, 0.0f, -1.0f,
                0.0f, 1.0f, 0.0f);
   }

   virtual void OnClose(void)
   {
      FreeImage_DeInitialise();
   }
   virtual void OnMouseDown(int button, int x, int y) {}
   virtual void OnMouseUp(int button, int x, int y) {}
   virtual void OnMouseWheel(int nWheelNumber, int nDirection, int x, int y) {}

   virtual void OnKeyDown(int nKey, char cAscii)
   {
      if (cAscii == 27) // 0x1b = ESC
      {
         this->Close(); // Close Window!
      }
   }

   virtual void OnKeyUp(int nKey, char cAscii)
   {
      if (cAscii == 's') // s: Shader
      {
         if (shaderPhong) shaderPhong->enable();
         if (shaderTex) shaderTex->enable();
      }
      else if (cAscii == 'f') // f: Fixed Function
      {
         if (shaderPhong) shaderPhong->disable();
         if (shaderTex) shaderTex->disable();
      }
      else if (cAscii == 'g') // g: toggle grid debug
      {
         showGrid = !showGrid;
      }
   }

   void UpdateTimer()
   {
      time1 = clock();
      float delta = static_cast<float>(static_cast<double>(time1 - time0) / static_cast<double>(CLOCKS_PER_SEC));
      delta = delta / 4;
      if (delta > 0.00005f)
      {
         time0 = clock();
         if (bUp)
         {
            timer010 += delta;
            if (timer010 >= 1.0f) { timer010 = 1.0f; bUp = false; }
         }
         else
         {
            timer010 -= delta;
            if (timer010 <= 0.0f) { timer010 = 0.0f; bUp = true; }
         }
      }
   }

   void DemoLight(void)
   {
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      glEnable(GL_NORMALIZE);

      GLfloat lmKa[] = { 0.0f, 0.0f, 0.0f, 0.0f };
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmKa);

      glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f);
      glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0f);

      GLfloat spot_direction[] = { 1.0f, -1.0f, -1.0f };
      GLint spot_exponent = 30;
      GLint spot_cutoff = 180;

      glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
      glLighti(GL_LIGHT0, GL_SPOT_EXPONENT, spot_exponent);
      glLighti(GL_LIGHT0, GL_SPOT_CUTOFF, spot_cutoff);

      GLfloat Kc = 1.0f;
      GLfloat Kl = 0.0f;
      GLfloat Kq = 0.0f;

      glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, Kc);
      glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, Kl);
      glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, Kq);

      GLfloat light_pos[] = { 0.0f, 5.0f, 5.0f, 1.0f };
      GLfloat light_Ka[] = { 1.0f, 0.5f, 0.5f, 1.0f };
      GLfloat light_Kd[] = { 1.0f, 0.1f, 0.1f, 1.0f };
      GLfloat light_Ks[] = { 1.0f, 1.0f, 1.0f, 1.0f };

      glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
      glLightfv(GL_LIGHT0, GL_AMBIENT, light_Ka);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, light_Kd);
      glLightfv(GL_LIGHT0, GL_SPECULAR, light_Ks);

      GLfloat material_Ka[] = { 0.5f, 0.0f, 0.0f, 1.0f };
      GLfloat material_Kd[] = { 0.4f, 0.4f, 0.5f, 1.0f };
      GLfloat material_Ks[] = { 0.8f, 0.8f, 0.0f, 1.0f };
      GLfloat material_Ke[] = { 0.1f, 0.0f, 0.0f, 0.0f };
      GLfloat material_Se = 20.0f;

      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
      glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);
   }
};

//-----------------------------------------------------------------------------

class myApplication : public cwc::glApplication
{
public:
   virtual void OnInit() { std::cout << "Hello World!\n"; }
};

//-----------------------------------------------------------------------------

int main(void)
{
   myApplication* pApp = new myApplication;
   myWindow* myWin = new myWindow();
   (void)myWin;

   pApp->run();
   delete pApp;
   return 0;
}

//-----------------------------------------------------------------------------

