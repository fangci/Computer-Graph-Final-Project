#include <iostream>
#include "context.h"
#include "program.h"
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


GLuint VBO;
GLuint cubemapTexture;
float skyboxVertices[] = {
    // Right face (X positive)
    1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,

    // Left face (X negative)
    -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
    1.0f,

    // Top face (Y positive)
    -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

    // Bottom face (Y negative)
    -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    1.0f,

    // Front face (Z positive)
    -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,

    // Back face (Z negative)
    -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
    -1.0f};

GLuint loadCubemap(const std::vector<std::string>& faces) {
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

bool SkyboxProgram::load() { 
    programId = quickCreateProgram(vertProgramFile, fragProgramFIle);
    VAO = new GLuint;
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(*VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    std::vector<std::string> faces{"../assets/models/skybox/front.jpg", "../assets/models/skybox/back.jpg",
                                   "../assets/models/skybox/bottom.jpg",   "../assets/models/skybox/top.jpg",
                                   "../assets/models/skybox/right.jpg", "../assets/models/skybox/left.jpg"};
    cubemapTexture = loadCubemap(faces);
    return programId != 0;
}

void SkyboxProgram::doMainLoop() {  
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);  
    glUseProgram(programId);  

    static float rotationangle = 0.0f;
    rotationangle += 0.005f;
    if (rotationangle > 360.0f) rotationangle -= 360.0f;

    glm::mat4 view = glm::mat4(glm::mat3(glm::make_mat4(ctx->camera->getViewMatrix())));
    view = glm::rotate(view, glm::radians(rotationangle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::make_mat4(ctx->camera->getProjectionMatrix());  

    glUniformMatrix4fv(glGetUniformLocation(programId, "view"), 1, GL_FALSE, glm::value_ptr(view));  
    glUniformMatrix4fv(glGetUniformLocation(programId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));  

    float speed = 10.0f;
    float sunAngle = glfwGetTime() * speed;

    glm::vec3 lightDir =
        glm::normalize(glm::vec3(0.0f, sin(glm::radians(sunAngle)) * 10.0f, -cos(glm::radians(sunAngle)) * 10.0f));
    float heightFactor = sin(glm::radians(sunAngle));
    glm::vec3 lightColor = glm::mix(glm::vec3(0.5f, 0.3f, 0.15f), glm::vec3(0.5f, 0.5f, 0.5f), heightFactor);
    glm::vec3 ambientColor = glm::mix(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f), heightFactor);

    glm::vec3 horizonColor = glm::mix(glm::vec3(0.5f, 0.3f, 0.15f), glm::vec3(1.0f, 1.0f, 1.0f), heightFactor);
    glm::vec3 zenithColor = glm::mix(glm::vec3(0.4f, 0.8f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), heightFactor);

    glUniform3fv(glGetUniformLocation(programId, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(programId, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(programId, "horizonColor"), 1, glm::value_ptr(horizonColor));
    glUniform3fv(glGetUniformLocation(programId, "zenithColor"), 1, glm::value_ptr(zenithColor));

    glBindVertexArray(*VAO);  
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);  
    glDrawArrays(GL_TRIANGLES, 0, 36);  
    glDepthFunc(GL_LESS);  
    glEnable(GL_CULL_FACE);
}
