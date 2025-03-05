#include "model.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <glm/vec3.hpp>

Model* Model::fromObjectFile(const char* obj_file) {
  Model* m = new Model();
  std::ifstream ObjFile(obj_file);

  if (!ObjFile.is_open()) {
    std::cout << "Can't open File !" << std::endl;
    return NULL;
  }

  /* TODO#1: Load model data from OBJ file
   *         You only need to handle v, vt, vn, f
   *         Other fields you can directly ignore
   *         Fill data into m->positions, m->texcoords m->normals and m->numVertex
   *         Data format:
   *           For positions and normals
   *         | 0    | 1    | 2    | 3    | 4    | 5    | 6    | 7    | 8    | 9    | 10   | 11   | ...
   *         | face 1                                                       | face 2               ...
   *         | v1x  | v1y  | v1z  | v2x  | v2y  | v2z  | v3x  | v3y  | v3z  | v1x  | v1y  | v1z  | ...
   *         | vn1x | vn1y | vn1z | vn1x | vn1y | vn1z | vn1x | vn1y | vn1z | vn1x | vn1y | vn1z | ...
   *           For texcoords
   *         | 0    | 1    | 2    | 3    | 4    | 5    | 6    | 7    | ...
   *         | face 1                                  | face 2        ...
   *         | v1x  | v1y  | v2x  | v2y  | v3x  | v3y  | v1x  | v1y  | ...
   * Note:
   *        OBJ File Format (https://en.wikipedia.org/wiki/Wavefront_.obj_file)
   *        Vertex per face = 3 or 4
   */
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> normals;
  std::string line;
  while (std::getline(ObjFile, line)) {
    std::istringstream iss(line);
    std::string type;
    iss >> type;
    if (type == "v") {
      glm::vec3 pos;
      iss >> pos.x >> pos.y >> pos.z;
      positions.push_back(pos);
    } else if (type == "vt") {
      glm::vec2 tex;
      iss >> tex.x >> tex.y;
      texcoords.push_back(tex);
    } else if (type == "vn") {
      glm::vec3 normal;
      iss >> normal.x >> normal.y >> normal.z;
      normals.push_back(normal);
    } else if (type == "f") {
      std::string face;
      while(iss >> face) {
        std::istringstream face_iss(face);
        std::string vertex, texcoord, normal;
        std::getline(face_iss, vertex, '/');
        std::getline(face_iss, texcoord, '/');
        std::getline(face_iss, normal, '/');
        m->positions.push_back(positions[std::stoi(vertex) - 1].x);
        m->positions.push_back(positions[std::stoi(vertex) - 1].y);
        m->positions.push_back(positions[std::stoi(vertex) - 1].z);
        m->texcoords.push_back(texcoords[std::stoi(texcoord) - 1].x);
        m->texcoords.push_back(texcoords[std::stoi(texcoord) - 1].y);
        m->normals.push_back(normals[std::stoi(normal) - 1].x);
        m->normals.push_back(normals[std::stoi(normal) - 1].y);
        m->normals.push_back(normals[std::stoi(normal) - 1].z);
      }
    }
  }
  m->numVertex = m->positions.size() / 3;
  return m;
}
