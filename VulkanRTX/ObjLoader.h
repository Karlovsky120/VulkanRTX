/*#pragma once

#include "Vertex.h"

#include "glm/vec3.hpp"

#include <string>
#include <vector>

struct FaceCoeff {
	glm::vec3 normal;
	float d;
};

struct ObjectData {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<FaceCoeff> faceCoeffs;
};

class ObjLoader {
public:
	static ObjectData loadObj(std::string modelPath);
	static glm::vec3 getVertexByIndex(ObjectData& object, uint32_t index);

private:
	static void getVertex(std::string& line, std::vector<Vertex>& vertices);
	static void getIndices(std::string& line, std::vector<uint32_t>& indices);
	static void normalize(ObjectData& object);
	static void calculateSurfaceNormals(ObjectData& object);
};*/

