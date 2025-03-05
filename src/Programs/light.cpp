#include <iostream>
#include "context.h"
#include "program.h"

bool LightProgram::load() {
  programId = quickCreateProgram(vertProgramFile, fragProgramFIle);
  int num_model = (int)ctx->models.size();
  VAO = new GLuint[num_model];

  glGenVertexArrays(num_model, VAO);
  for (int i = 0; i < num_model; i++) {
    glBindVertexArray(VAO[i]);
    Model* model = ctx->models[i];

    GLuint VBO[3];
    glGenBuffers(3, VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->positions.size(), model->positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->normals.size(), model->normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->texcoords.size(), model->texcoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  }
  return programId != 0;
}

void LightProgram::doMainLoop() {
  // add sunlight
  float speed = 10.0f;
  float sunAngle = glfwGetTime() * speed;

  glm::vec3 lightDir =
      glm::normalize(glm::vec3(cos(glm::radians(sunAngle)) * 10.0f, sin(glm::radians(sunAngle)) * 10.0f, 0.0f));
  float heightFactor = sin(glm::radians(sunAngle));
  glm::vec3 lightColor = glm::mix(glm::vec3(0.5f, 0.3f, 0.15f), glm::vec3(0.5f, 0.5f, 0.5f), heightFactor);
  glm::vec3 ambientColor = glm::mix(glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f), heightFactor);

  int obj_num = (int)ctx->objects.size();
  for (int i = 0; i < obj_num; i++) {
    int modelIndex = ctx->objects[i]->modelIndex;
    GLint programId = ctx->programs[ctx->objects[i]->programId]->programId;
    glUseProgram(programId);
    glBindVertexArray(VAO[modelIndex]);

    Model* model = ctx->models[modelIndex];
    const float* p = ctx->camera->getProjectionMatrix();
    GLint pmatLoc = glGetUniformLocation(programId, "Projection");
    glUniformMatrix4fv(pmatLoc, 1, GL_FALSE, p);

    const float* v = ctx->camera->getViewMatrix();
    GLint vmatLoc = glGetUniformLocation(programId, "ViewMatrix");
    glUniformMatrix4fv(vmatLoc, 1, GL_FALSE, v);

    const float* m = glm::value_ptr(ctx->objects[i]->transformMatrix * model->modelMatrix);
    GLint mmatLoc = glGetUniformLocation(programId, "ModelMatrix");
    glUniformMatrix4fv(mmatLoc, 1, GL_FALSE, m);

    glm::mat4 TIMatrix = glm::transpose(glm::inverse(model->modelMatrix));
    const float* ti = glm::value_ptr(TIMatrix);
    mmatLoc = glGetUniformLocation(programId, "TIModelMatrix");
    glUniformMatrix4fv(mmatLoc, 1, GL_FALSE, ti);

    const float* vp = ctx->camera->getPosition();
    mmatLoc = glGetUniformLocation(programId, "viewPos");
    glUniform3fv(mmatLoc, 1, vp);

    // sunlight
    glUniform3fv(glGetUniformLocation(programId, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(programId, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(programId, "ambientColor"), 1, glm::value_ptr(ambientColor));

    if (ctx->objects[i]->programId == ctx->terrainProgramIndex) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, model->textures[0]);  // 低地貼圖
      glUniform1i(glGetUniformLocation(programId, "lowTex"), 0);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, model->textures[1]);  // 高地貼圖
      glUniform1i(glGetUniformLocation(programId, "highTex"), 1);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, model->textures[2]);  // 水中貼圖
      glUniform1i(glGetUniformLocation(programId, "Water"), 2);

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, displacementMap);  // 高度圖
      glUniform1i(glGetUniformLocation(programId, "displacementMap"), 3);

    } else if (ctx->objects[i]->programId == ctx->OceanProgramIndex) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glUniform1f(glGetUniformLocation(programId, "time"), glfwGetTime());

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, model->textures[ctx->objects[i]->textureIndex]);
      glUniform1i(glGetUniformLocation(programId, "ourTexture"), 0);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, model->textures[1]);
      glUniform1i(glGetUniformLocation(programId, "mossTexture"), 1);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, displacementMap);
      glUniform1i(glGetUniformLocation(programId, "displacementMap"), 2);
      GLint location = glGetUniformLocation(programId, "displacementMap");
      lightColor = glm::mix(glm::vec3(0.5f, 0.3f, 0.15f), glm::vec3(0.5f, 0.5f, 0.5f), std::abs(heightFactor));
      glUniform3fv(glGetUniformLocation(programId, "lightColor"), 1, glm::value_ptr(lightColor));
      glUniform1f(glGetUniformLocation(programId, "amplitude"), 10.0f);
      glUniform3f(glGetUniformLocation(programId, "waterColor"), 0.3f, 0.8f, 1.0f);
      glUniform3f(glGetUniformLocation(programId, "lightPos"), ctx->directionLightDirection.x,
                  ctx->directionLightDirection.y, ctx->directionLightDirection.z);
      glBindVertexArray(VAO[ctx->objects[i]->modelIndex]);
    } 
    else if (ctx->objects[i]->programId == ctx->plantsProgramIndex) {
      glUniform1f(glGetUniformLocation(programId, "time"), glfwGetTime());
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, model->textures[ctx->objects[i]->textureIndex]);
      glUniform1i(glGetUniformLocation(programId, "ourTexture"), 0);
    } 
    else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model->textures[ctx->objects[i]->textureIndex]);
        glUniform1i(glGetUniformLocation(programId, "ourTexture"), 0);
        glBindVertexArray(VAO[ctx->objects[i]->modelIndex]);
    }
    glDrawArrays(model->drawMode, 0, model->numVertex);
  }
  glUseProgram(0);
}
