#include <iostream>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "OBJloader.h"

#define MAX_LINE_SIZE 255

bool loadOBJ(const std::filesystem::path& path, std::vector < glm::vec3 >& out_vertices, std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals)
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    out_vertices.clear();
    out_uvs.clear();
    out_normals.clear();

    FILE* file;
    fopen_s(&file, path.string().c_str(), "r");
    if (file == NULL) {
        printf("Impossible to open the file !\n");
        return false;
    }
    int line = 0;

    while (1) {
        line++;
        char lineHeader[MAX_LINE_SIZE];
        int res = fscanf_s(file, "%s", lineHeader, MAX_LINE_SIZE);
        if (res == EOF) {
            break;
        }

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf_s(file, "%f %f\n", &uv.y, &uv.x);
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
            int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3]);
            if (matches == 12) {
                for (int i = 0; i < 3; i++) {
                    vertexIndices.push_back(vertexIndex[i]);
                    uvIndices.push_back(uvIndex[i]);
                    normalIndices.push_back(normalIndex[i]);
                }
                vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[2]);

                vertexIndices.push_back(vertexIndex[3]);
                uvIndices.push_back(uvIndex[3]);
                normalIndices.push_back(normalIndex[3]);

                vertexIndices.push_back(vertexIndex[0]);
                uvIndices.push_back(uvIndex[0]);
                normalIndices.push_back(normalIndex[0]);
                continue;
            }
            if (matches != 9) {
                printf("File can't be read by simple parser :( Try exporting with other options\n");
                return false;
            }
            for (int i = 0; i < 3; i++) {
                vertexIndices.push_back(vertexIndex[i]);
                uvIndices.push_back(uvIndex[i]);
                normalIndices.push_back(normalIndex[i]);
            }
        }
    }

    // unroll from indirect to direct vertex specification
    // sometimes not necessary, definitely not optimal
    out_vertices.reserve(vertexIndices.size());
    out_uvs.reserve(uvIndices.size());
    out_normals.reserve(normalIndices.size());

    for (unsigned int vertexIndex : vertexIndices) {
        out_vertices.push_back(temp_vertices[vertexIndex - 1]);
    }
    for (unsigned int uvIndex : uvIndices) {
        out_uvs.push_back(temp_uvs[uvIndex - 1]);
    }
    for (unsigned int normalIndex : normalIndices) {
        out_normals.push_back(temp_normals[normalIndex - 1]);
    }

    fclose(file);
    return true;
}