#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include <GLFW/glfw3.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION
#include <glm/glm.hpp>

#include <glm/ext/matrix_transform.hpp>

#include "camera.h"
#include "context.h"
#include "gl_helper.h"
#include "model.h"
#include "opengl_context.h"
#include "program.h"
#include "utils.h"

#include <fftw3.h>
#include <complex>
#include <glm/gtc/noise.hpp>
#include <random>

void initOpenGL();
void resizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int, int action, int);

Context ctx;

Material mFlatwhite;
Material mShinyred;
Material mClearblue;

GLuint displacementMap;
const int oceanWidth = 128;
const int oceanHeight = 128;
const float gravity = 9.81f;
std::vector<std::complex<float>> h0(oceanWidth* oceanHeight);

// FFTW plan
fftwf_plan fftPlan;
std::vector<float> displacement(oceanWidth* oceanHeight);

void initializeFFTResources() {
  fftPlan = fftwf_plan_dft_c2r_2d(oceanHeight, oceanWidth, reinterpret_cast<fftwf_complex*>(h0.data()),
                                  displacement.data(), FFTW_ESTIMATE);

  fftwf_execute(fftPlan);
}

void destroyFFTResources() { fftwf_destroy_plan(fftPlan); }

float PhillipsSpectrum(float kx, float ky) {
  glm::vec2 windDirection = glm::normalize(glm::vec2(1.0f, 0.0f));  // 風的方向
  float windSpeed = 20.0f;                                          // 風速大小
  float A = 0.001f;                                                 // 調節參數
  float g = gravity;

  glm::vec2 k = glm::vec2(kx, ky);
  float k_length = glm::length(k);
  if (k_length < 0.0001f) return 0.0f;

  float L = windSpeed * windSpeed / g;  // 特徵波長
  float k_dot_w = glm::dot(glm::normalize(k), windDirection);

  float phillips = A * exp(-1.0f / (k_length * L) * (k_length * L)) / (k_length * k_length * k_length * k_length);
  phillips *= k_dot_w * k_dot_w;

  // 限制最小值避免不穩定
  phillips = glm::min(phillips, 1e-3f);

  return phillips;
}

void initializeWaveSpectrum() {
  for (int y = 0; y < oceanHeight; ++y) {
    for (int x = 0; x < oceanWidth; ++x) {
      float kx = (x - oceanWidth / 2.0f) / oceanWidth;
      float ky = (y - oceanHeight / 2.0f) / oceanHeight;
      float amplitude = sqrt(PhillipsSpectrum(kx, ky));
      float phase = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
      h0[y * oceanWidth + x] = std::polar(amplitude, phase);
    }
  }
}

void createFFTDisplacementMap() {
  glGenTextures(1, &displacementMap);
  glBindTexture(GL_TEXTURE_2D, displacementMap);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, oceanWidth, oceanHeight, 0, GL_RED, GL_FLOAT, nullptr);
}

void updateFFTDisplacementMap(float time) {
  std::vector<std::complex<float>> ht(oceanWidth * oceanHeight);

  for (int y = 0; y < oceanHeight; ++y) {
    for (int x = 0; x < oceanWidth; ++x) {
      float kx = (x < oceanWidth / 2) ? (2.0f * M_PI * x / oceanWidth) : (-2.0f * M_PI * (oceanWidth - x) / oceanWidth);
      float ky =
          (y < oceanHeight / 2) ? (2.0f * M_PI * y / oceanHeight) : (-2.0f * M_PI * (oceanHeight - y) / oceanHeight);

      float omega = sqrt(gravity * glm::length(glm::vec2(kx, ky)));
      ht[y * oceanWidth + x] = h0[y * oceanWidth + x] * exp(std::complex<float>(0, omega * time));
    }
  }
  fftwf_execute_dft_c2r(fftPlan, (fftwf_complex*)ht.data(), displacement.data());
  for (size_t i = 0; i < displacement.size(); ++i) {
    displacement[i] /= (oceanWidth * oceanHeight);
  }
  glBindTexture(GL_TEXTURE_2D, displacementMap);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, oceanWidth, oceanHeight, GL_RED, GL_FLOAT, displacement.data());
}

void loadMaterial() {
  mFlatwhite.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
  mFlatwhite.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
  mFlatwhite.specular = glm::vec3(0.0f, 0.0f, 0.0f);
  mFlatwhite.shininess = 10;

  mShinyred.ambient = glm::vec3(0.1985f, 0.0000f, 0.0000f);
  mShinyred.diffuse = glm::vec3(0.5921f, 0.0167f, 0.0000f);
  mShinyred.specular = glm::vec3(0.5973f, 0.2083f, 0.2083f);
  mShinyred.shininess = 100.0f;

  mClearblue.ambient = glm::vec3(0.0394f, 0.0394f, 0.3300f);
  mClearblue.diffuse = glm::vec3(0.1420f, 0.1420f, 0.9500f);
  mClearblue.specular = glm::vec3(0.1420f, 0.1420f, 0.9500f);
  mClearblue.shininess = 10;
}

void loadPrograms() {
  ctx.programs.push_back(new ExampleProgram(&ctx));
  ctx.programs.push_back(new LightProgram(&ctx));
  ctx.programs[1]->vertProgramFile = "../assets/shaders/terrain.vert";
  ctx.programs[1]->fragProgramFIle = "../assets/shaders/terrain.frag";

  ctx.programs.push_back(new LightProgram(&ctx));
  ctx.programs[2]->vertProgramFile = "../assets/shaders/ocean.vert";
  ctx.programs[2]->fragProgramFIle = "../assets/shaders/ocean.frag";

  ctx.programs.push_back(new LightProgram(&ctx));
  ctx.programs[3]->vertProgramFile = "../assets/shaders/grass.vert";
  ctx.programs[3]->fragProgramFIle = "../assets/shaders/grass.frag";

  ctx.programs.push_back(new SkyboxProgram(&ctx));

  for (auto iter = ctx.programs.begin(); iter != ctx.programs.end(); iter++) {
    if (!(*iter)->load()) {
      std::cout << "Load program fail, force terminate" << std::endl;
      exit(1);
    }
  }
  glUseProgram(0);
}

std::vector<std::vector<float>> generateHeightMap(int width, int height, float scale) {
  std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));
  float centerX = 38.0f;
  float centerY = 38.0f;
  float maxRadius = 25.0f;

  for (int z = 0; z < height; ++z) {
    for (int x = 0; x < width; ++x) {
      // 計算距離中心的歐幾里得距離
      float distance = glm::distance(glm::vec2(x, z), glm::vec2(centerX, centerY));

      // 應用圓形遮罩：距離超過半徑時設置為 0
      if (distance <= maxRadius) {
        float mask = 1.0f - (distance / maxRadius);  // 距離越遠，值越小
        heightMap[z][x] = glm::perlin(glm::vec2(x * scale, z * scale)) * mask;
      } else {
        heightMap[z][x] = 0.0f;  // 超出範圍的地方設置為 0
      }
    }
  }
  return heightMap;
}

std::vector<std::vector<float>> heightMap = generateHeightMap(77, 77, 0.1f);
std::vector<std::vector<glm::vec3>> normalMap(77, std::vector<glm::vec3>(77, glm::vec3(0.0f)));

// 隨機生成位置向量
glm::vec3 generateRandomPosition(float xMin, float xMax, float zMin, float zMax) {
  // 隨機數生成器
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> xDist(xMin, xMax);
  std::uniform_real_distribution<float> zDist(zMin, zMax);
  int x = xDist(gen);
  int z = zDist(gen);
  float y = heightMap[z][x] * 20;
  while (y <= 0.1 || y>=1)  {
    x = xDist(gen);
    z = zDist(gen);
    y = heightMap[z][x] * 20;
  }
  // 隨機生成 x, y, z 值
  return glm::vec3(x, y, z);
}

// 隨機生成比例向量
glm::vec3 generateRandomScale(float minScale, float maxScale) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> scaleDist(minScale, maxScale);

  // 為每個分量生成隨機比例
  float scaleX = scaleDist(gen);
  float scaleY = scaleDist(gen);
  float scaleZ = scaleDist(gen);

  return glm::vec3(scaleX, scaleY, scaleZ);
}

Model* createIsland() {
  Model* m = new Model();

  int width = 76;
  int height = 76;
  int scale = 20;

  std::vector<glm::vec3> normalsAccum((width + 1) * (height + 1), glm::vec3(0.0f));
  std::vector<int> normalCounts((width + 1) * (height + 1), 0);

  // 使用 Perlin 噪聲生成高度
  for (int z = height - 1; z >= 0; --z) {
    for (int x = 0; x < width; ++x) {
      float x1 = x - width / 2;
      float z1 = z - height / 2;
      float x2 = x1 + 1;
      float z2 = z1 + 1;
      float y1 = heightMap[z][x] * scale;
      float y2 = heightMap[z + 1][x] * scale;
      float y3 = heightMap[z][x + 1] * scale;
      float y4 = heightMap[z + 1][x + 1] * scale;

      m->positions.push_back(x1);
      m->positions.push_back(y1);
      m->positions.push_back(z1);
      m->positions.push_back(x1);
      m->positions.push_back(y2);
      m->positions.push_back(z2);
      m->positions.push_back(x2);
      m->positions.push_back(y3);
      m->positions.push_back(z1);

      m->texcoords.push_back(x1 / (float)width * 10);
      m->texcoords.push_back(z1 / (float)height * 10);
      m->texcoords.push_back(x1 / (float)width * 10);
      m->texcoords.push_back(z2 / (float)height * 10);
      m->texcoords.push_back(x2 / (float)width * 10);
      m->texcoords.push_back(z1 / (float)height * 10);

      m->positions.push_back(x1);
      m->positions.push_back(y2);
      m->positions.push_back(z2);
      m->positions.push_back(x2);
      m->positions.push_back(y4);
      m->positions.push_back(z2);
      m->positions.push_back(x2);
      m->positions.push_back(y3);
      m->positions.push_back(z1);

      m->texcoords.push_back(x1 / (float)width * 10);
      m->texcoords.push_back(z2 / (float)height * 10);
      m->texcoords.push_back(x2 / (float)width * 10);
      m->texcoords.push_back(z2 / (float)height * 10);
      m->texcoords.push_back(x2 / (float)width * 10);
      m->texcoords.push_back(z1 / (float)height * 10);

      // 第一個三角形
      glm::vec3 normal1 = glm::cross(glm::vec3(x1 - x1, y1 - y2, z1 - z2), glm::vec3(x1 - x2, y1 - y3, z1 - z1));
      if (glm::length(normal1) < 0.0001f) {
        normal1 = glm::vec3(0.0f, 1.0f, 0.0f);
      } else {
        normal1 = glm::normalize(normal1);
      }
      int index1 = z * width + x;
      int index2 = (z + 1) * width + x;
      int index3 = z * width + (x + 1);

      normalsAccum[index1] += normal1;
      normalsAccum[index2] += normal1;
      normalsAccum[index3] += normal1;

      normalCounts[index1]++;
      normalCounts[index2]++;
      normalCounts[index3]++;

      // 第二個三角形
      glm::vec3 normal2 = glm::cross(glm::vec3(x1 - x2, y2 - y4, z2 - z2), glm::vec3(x1 - x2, y2 - y3, z2 - z1));
      if (glm::length(normal2) < 0.0001f) {
        normal2 = glm::vec3(0.0f, 1.0f, 0.0f);
      } else {
        normal2 = glm::normalize(normal2);
      }
      normalsAccum[index2] += normal2;
      normalsAccum[(z + 1) * width + (x + 1)] += normal2;
      normalsAccum[index3] += normal2;

      normalCounts[index2]++;
      normalCounts[(z + 1) * width + (x + 1)]++;
      normalCounts[index3]++;
    }
  }

  for (int z = height - 1; z >= 0; --z) {
    for (int x = 0; x < width; ++x) {
      int index = z * width + x;
      if (normalCounts[index] > 0) {
        glm::vec3 averageNormal = glm::normalize(normalsAccum[index] / (float)normalCounts[index]);
        m->normals.push_back(averageNormal.x);
        m->normals.push_back(averageNormal.y);
        m->normals.push_back(averageNormal.z);
        normalMap[z][x] = averageNormal;
      }
      index = (z + 1) * width + x;
      if (normalCounts[index] > 0) {
        glm::vec3 averageNormal = glm::normalize(normalsAccum[index] / (float)normalCounts[index]);
        m->normals.push_back(averageNormal.x);
        m->normals.push_back(averageNormal.y);
        m->normals.push_back(averageNormal.z);
        normalMap[z + 1][x] = averageNormal;
      }
      index = z * width + (x + 1);
      if (normalCounts[index] > 0) {
        glm::vec3 averageNormal = glm::normalize(normalsAccum[index] / (float)normalCounts[index]);
        m->normals.push_back(averageNormal.x);
        m->normals.push_back(averageNormal.y);
        m->normals.push_back(averageNormal.z);
        normalMap[z][x + 1] = averageNormal;
      }
      index = (z + 1) * width + x;
      if (normalCounts[index] > 0) {
        glm::vec3 averageNormal = glm::normalize(normalsAccum[index] / (float)normalCounts[index]);
        m->normals.push_back(averageNormal.x);
        m->normals.push_back(averageNormal.y);
        m->normals.push_back(averageNormal.z);
        normalMap[z + 1][x] = averageNormal;
      }
      index = (z + 1) * width + (x + 1);
      if (normalCounts[index] > 0) {
        glm::vec3 averageNormal = glm::normalize(normalsAccum[index] / (float)normalCounts[index]);
        m->normals.push_back(averageNormal.x);
        m->normals.push_back(averageNormal.y);
        m->normals.push_back(averageNormal.z);
        normalMap[z + 1][x + 1] = averageNormal;
      }
      index = z * width + (x + 1);
      if (normalCounts[index] > 0) {
        glm::vec3 averageNormal = glm::normalize(normalsAccum[index] / (float)normalCounts[index]);
        m->normals.push_back(averageNormal.x);
        m->normals.push_back(averageNormal.y);
        m->normals.push_back(averageNormal.z);
        normalMap[z][x + 1] = averageNormal;
      }
    }
  }

  // 設置模型參數
  m->numVertex = m->positions.size() / 3;
  m->drawMode = GL_TRIANGLES;
  m->textures.push_back(createTexture("../assets/models/terrain/moss.jpg"));
  m->textures.push_back(createTexture("../assets/models/terrain/stone.jpg"));
  m->textures.push_back(createTexture("../assets/models/ocean/water.jpg"));
  m->modelMatrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(1, 1, 1));
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // 啟用線框模式
  // glDisable(GL_CULL_FACE);                    // 關閉背面剔除

  return m;
}

Model* createOcean(float size, float waterLevel) {
  Model* m = new Model();

  // 如果地勢低於0.1，生成海洋
  for (int z = 0; z < size; ++z) {
    for (int x = 0; x < size; ++x) {
      float x1 = x - size / 2;
      float z1 = z - size / 2;
      float x2 = x1 + 1;
      float z2 = z1 + 1;

      m->positions.push_back(x1);
      m->positions.push_back(waterLevel);
      m->positions.push_back(z1);
      m->positions.push_back(x1);
      m->positions.push_back(waterLevel);
      m->positions.push_back(z2);
      m->positions.push_back(x2);
      m->positions.push_back(waterLevel);
      m->positions.push_back(z1);

      for (int i = 0; i < 3; i++) {
        m->normals.push_back(0);
        m->normals.push_back(1);
        m->normals.push_back(0);
      }

      m->texcoords.push_back(x1 / (float)size * 10);
      m->texcoords.push_back(z1 / (float)size * 10);
      m->texcoords.push_back(x1 / (float)size * 10);
      m->texcoords.push_back(z2 / (float)size * 10);
      m->texcoords.push_back(x2 / (float)size * 10);
      m->texcoords.push_back(z1 / (float)size * 10);

      m->positions.push_back(x1);
      m->positions.push_back(waterLevel);
      m->positions.push_back(z2);
      m->positions.push_back(x2);
      m->positions.push_back(waterLevel);
      m->positions.push_back(z2);
      m->positions.push_back(x2);
      m->positions.push_back(waterLevel);
      m->positions.push_back(z1);

      for (int i = 0; i < 3; i++) {
        m->normals.push_back(0);
        m->normals.push_back(1);
        m->normals.push_back(0);
      }

      m->texcoords.push_back(x1 / (float)size * 10);
      m->texcoords.push_back(z2 / (float)size * 10);
      m->texcoords.push_back(x2 / (float)size * 10);
      m->texcoords.push_back(z2 / (float)size * 10);
      m->texcoords.push_back(x2 / (float)size * 10);
      m->texcoords.push_back(z1 / (float)size * 10);
    }
  }

  // 設置模型參數
  m->numVertex = m->positions.size() / 3;
  m->drawMode = GL_TRIANGLES;
  m->textures.push_back(createTexture("../assets/models/ocean/water.jpg"));
  m->textures.push_back(createTexture("../assets/models/terrain/moss.jpg"));

  return m;
}

Model* createPlants() {
  Model* tree = Model::fromObjectFile("../assets/models/grass/grass.obj");
  if (!tree) {
    std::cerr << "Error: Failed to load the dice model!" << std::endl;
    return nullptr;
  }
  GLuint textureID = createTexture("../assets/models/grass/grass.png");
  if (textureID == 0) {
    std::cerr << "Error: Failed to load dice.jpg! Please check the file path or format." << std::endl;
    delete tree;
    return nullptr;
  }
  tree->textures.push_back(textureID);
  tree->modelMatrix = glm::scale(tree->modelMatrix, glm::vec3(0.6f, 0.6f, 0.6f));
  tree->drawMode = GL_QUADS;
  return tree;
}

void loadModels() {
  ctx.models.push_back(createIsland());
  ctx.models.push_back(createOcean(75, 0.2f));
  ctx.models.push_back(createPlants());
}

void setupObjects() {
  Object* terrainObject = new Object(0, glm::mat4(1.0f));
  terrainObject->programId = ctx.terrainProgramIndex;
  ctx.objects.push_back(terrainObject);

  float xMin = 13.0f, xMax = 63.0f;
  float zMin = 13.0f, zMax = 63.0f;
  float scaleMin = 0.4f, scaleMax = 1.0f;

  for (int i = 0; i < 50; i++) {
    glm::vec3 position = generateRandomPosition(xMin, xMax, zMin, zMax) + glm::vec3(-38, 0, -38);
    glm::vec3 scale = generateRandomScale(scaleMin, scaleMax);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 islandNormal = normalMap[(int)position.z + 38][(int)position.x + 38];
    glm::vec3 angleAxis = glm::cross(up, islandNormal);
    angleAxis = glm::normalize(angleAxis);
    float angle = glm::acos(glm::dot(up, islandNormal)/glm::length(islandNormal));
    glm::quat rotation = glm::angleAxis(angle, angleAxis);
    glm::mat4 rotationMatrix = glm::mat4_cast(rotation);

    glm::vec3 adjustedPosition = position - 0.03f * islandNormal;
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), adjustedPosition);
    glm::mat4 modelMatrix = glm::scale(translationMatrix * rotationMatrix, scale);
    Object* plantsObject = new Object(2, modelMatrix);
    plantsObject->programId = ctx.plantsProgramIndex;
    ctx.objects.push_back(plantsObject);
  }

  Object* oceanObject = new Object(1, glm::mat4(1.0f));
  oceanObject->programId = ctx.OceanProgramIndex;
  ctx.objects.push_back(oceanObject);
}

int main() {
  initOpenGL();

  GLFWwindow* window = OpenGLContext::getWindow();
  glfwSetWindowTitle(window, "Final Project");

  // Init Camera helper
  Camera camera(glm::vec3(0, 2, 5));
  camera.initialize(OpenGLContext::getAspectRatio());
  // Store camera as glfw global variable for callbacks use
  glfwSetWindowUserPointer(window, &camera);
  ctx.camera = &camera;
  ctx.window = window;

  createFFTDisplacementMap();
  initializeWaveSpectrum();
  initializeFFTResources();
  loadMaterial();
  loadModels();
  loadPrograms();
  setupObjects();

  // Main rendering loop
  while (!glfwWindowShouldClose(window)) {
    // Polling events.
    glfwPollEvents();
    // Update camera position and view
    camera.move(window);
    // GL_XXX_BIT can simply "OR" together to use.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /// TO DO Enable DepthTest
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(1.0f);

    ctx.spotLightDirection = glm::normalize(glm::vec3(3, 0.3, 3) - ctx.spotLightPosition);
    ctx.pointLightPosition = glm::vec3(6 * glm::cos(glm::radians(ctx._pointLightPosisionDegree)), 3.0f,
                                       6 * glm::sin(glm::radians(ctx._pointLightPosisionDegree)));
    updateFFTDisplacementMap(glfwGetTime());
    ctx.programs[1]->doMainLoop();
    ctx.programs[4]->doMainLoop();

#ifdef __APPLE__
    // Some platform need explicit glFlush
    glFlush();
#endif
    glfwSwapBuffers(window);
  }
  return 0;
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
  // There are three actions: press, release, hold(repeat)
  // Press ESC to close the window.
  if (key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    return;
  }
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_F9: {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
          // Show the mouse cursor
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
          // Hide the mouse cursor
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        break;
      }
      default:
        break;
    }
  }
}

void resizeCallback(GLFWwindow* window, int width, int height) {
  OpenGLContext::framebufferResizeCallback(window, width, height);
  auto ptr = static_cast<Camera*>(glfwGetWindowUserPointer(window));
  if (ptr) {
    ptr->updateProjectionMatrix(OpenGLContext::getAspectRatio());
  }
}

void initOpenGL() {
  // Initialize OpenGL context, details are wrapped in class.
#ifdef __APPLE__
  // MacOS need explicit request legacy support
  OpenGLContext::createContext(21, GLFW_OPENGL_ANY_PROFILE);
#else
  OpenGLContext::createContext(21, GLFW_OPENGL_ANY_PROFILE);
//  OpenGLContext::createContext(43, GLFW_OPENGL_COMPAT_PROFILE);
#endif
  GLFWwindow* window = OpenGLContext::getWindow();
  glfwSetKeyCallback(window, keyCallback);
  glfwSetFramebufferSizeCallback(window, resizeCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#ifndef NDEBUG
  OpenGLContext::printSystemInfo();
  // This is useful if you want to debug your OpenGL API calls.
  OpenGLContext::enableDebugCallback();
#endif
}