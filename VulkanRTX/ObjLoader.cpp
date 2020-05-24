/*#include "ObjLoader.h"

#include "glm/vec3.hpp"
#include "glm/geometric.hpp"

#include <algorithm>
#include <fstream>

void ObjLoader::getVertex(std::string& line, std::vector<Vertex>& vertices) {
	uint32_t delimiterIndex = line.find(" ");
	line = line.substr(delimiterIndex + 1L);

	Vertex vertex;

	delimiterIndex = line.find(" ");
	vertex.position.x = std::stof(line.substr(0, delimiterIndex));

	line = line.substr(delimiterIndex + 1);
	delimiterIndex = line.find(" ");
	vertex.position.y = std::stof(line.substr(0, delimiterIndex));

	line = line.substr(delimiterIndex + 1);
	vertex.position.z = std::stof(line);

	vertices.push_back(vertex);
}

void ObjLoader::getIndices(std::string& line, std::vector<uint32_t>& indices) {
	line = line.substr(2);
	uint32_t delimiterIndex = line.find(" ");
	indices.push_back(std::stoi(line.substr(0, delimiterIndex)) - 1);

	line = line.substr(delimiterIndex + 1);
	delimiterIndex = line.find(" ");
	indices.push_back(std::stoi(line.substr(0, delimiterIndex)) - 1);

	line = line.substr(delimiterIndex + 1);
	indices.push_back(std::stoi(line) - 1);
}

void ObjLoader::normalize(ObjectData& object) {
	glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());

	for (Vertex& vertex : object.vertices) {
		max.x = std::max(max.x, vertex.position.x);
		min.x = std::min(min.x, vertex.position.x);
		max.y = std::max(max.y, vertex.position.y);
		min.y = std::min(min.y, vertex.position.y);
		max.z = std::max(max.z, vertex.position.z);
		min.z = std::min(min.z, vertex.position.z);
	}

	glm::vec3 mid = (min + max) * 0.5f;

	float maxScale = std::max(max.x - min.x, std::max(max.y - min.y, max.z - min.z));
	float scale = 2 / maxScale;

	for (Vertex& vertex : object.vertices) {
		vertex.position = (vertex.position - mid) * scale;
	}
}

void ObjLoader::calculateSurfaceNormals(ObjectData& object) {
	for (uint32_t i = 0; i < object.indices.size(); i += 3) {
		Vertex& first = object.vertices[object.indices[i]];
		Vertex& second = object.vertices[object.indices[i + 1]];
		Vertex& third = object.vertices[object.indices[i + 2]];

		glm::vec3 normal =
			glm::cross(second.position - first.position, third.position - first.position);

		first.normal += normal;
		second.normal += normal;
		third.normal += normal;
	}

	for (Vertex& vertex : object.vertices) {
		vertex.normal = glm::normalize(vertex.normal);
	}
}

glm::vec3 ObjLoader::getVertexByIndex(ObjectData& object, uint32_t index) {
	return object.vertices[object.indices[index]].position;
}

ObjectData ObjLoader::loadObj(std::string modelPath) {
	ObjectData object;

	std::ifstream modelFile(modelPath);
	std::string line;

	if (modelFile.is_open()) {
		while (getline(modelFile, line)) {
			if (line[0] == 'v') {
				getVertex(line, object.vertices);
			}
			else if (line[0] == 'f') {
				getIndices(line, object.indices);
			}
		}

		normalize(object);

		for (uint32_t i = 0; i < object.indices.size(); i += 3) {
			glm::vec3 first = object.vertices[object.indices[i]].position;
			glm::vec3 second = object.vertices[object.indices[i + 1]].position;
			glm::vec3 third = object.vertices[object.indices[i + 2]].position;

			glm::vec3 one = second - first;
			glm::vec3 two = third - first;
			glm::vec3 normal = glm::cross(one, two);
			float d = -glm::dot(normal, first);

			object.faceCoeffs.push_back({ normal, d });
		}

		calculateSurfaceNormals(object);

		if (modelPath == "kocka.obj") {
			glm::vec3 perfect = glm::vec3(0.57735026919f);
			for (Vertex& vertex : object.vertices) {
				vertex.normal = vertex.position * perfect;
			}
		}
	}

	return object;
}*/