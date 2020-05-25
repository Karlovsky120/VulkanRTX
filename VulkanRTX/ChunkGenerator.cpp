#include "ChunkGenerator.h"

#include <glm/vec3.hpp>

#include <chrono>
#include <iostream>
#include <future>
#include <stdint.h>
#include <stdlib.h>
#include <thread>

std::vector<Vertex> ChunkGenerator::generateVertices() {
	std::vector<Vertex> vertices;
	for (uint32_t i = 0; i < VERTEX_SIZE; ++i) {
		for (uint32_t j = 0; j < VERTEX_SIZE; ++j) {
			for (uint32_t k = 0; k < VERTEX_SIZE; ++k) {
				vertices.push_back({
					glm::vec3(
						i - static_cast<float>(CHUNK_SIZE) * 0.5,
						j - static_cast<float>(CHUNK_SIZE) * 0.5,
						k - static_cast<float>(CHUNK_SIZE) * 0.5
					),
					});
			}
		}
	}

	return vertices;
}

std::vector<uint32_t> ChunkGenerator::generateChunk(uint32_t seed) {
	bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE] = {};
	generateRandomly(seed, blocks);
	return generateGreedyTrianglesMultithreaded(blocks);
}

void ChunkGenerator::generateRandomly(uint32_t seed, bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE]) {
	srand(seed);

	for (uint32_t i = 1; i < CHUNK_SIZE + 1; ++i) {
		srand(rand());
		for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
			for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
				blocks[i][j][k] = rand() % 2 == 0;
			}
		}
	}
}

const inline uint32_t ChunkGenerator::getBlockIndex(uint32_t x, uint32_t y, uint32_t z) const {
	return x * CHUNK_PADDED_SIZE_SQUARED + y * CHUNK_PADDED_SIZE + z;
}

const inline uint32_t ChunkGenerator::getVertexIndex(uint32_t x, uint32_t y, uint32_t z) const {
	return x * VERTEX_SIZE_SQUARED + y * VERTEX_SIZE + z;
}

const inline void ChunkGenerator::pushRectangle(std::vector<uint32_t>& v, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4) const {
	v.push_back(v1);
	v.push_back(v2);
	v.push_back(v3);
	v.push_back(v2);
	v.push_back(v4);
	v.push_back(v3);
}

std::vector<uint32_t> ChunkGenerator::generateSimpleTriangles(bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE]) {
	std::vector<uint32_t> indices;

	for (uint32_t i = 1; i < CHUNK_SIZE + 1; ++i) {
		for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
			for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
				if (blocks[i][j][k] == 1) {
					uint32_t v1;
					uint32_t v3;
					uint32_t v2;
					uint32_t v4;

					if (blocks[i - 1][j][k] == 0) {
						v1 = getVertexIndex(i - 1, j - 1, k - 1);
						v3 = getVertexIndex(i - 1, j, k - 1);
						v2 = getVertexIndex(i - 1, j - 1, k);
						v4 = getVertexIndex(i - 1, j, k);

						pushRectangle(indices, v1, v2, v3, v4);
					}

					if (blocks[i + 1][j][k] == 0) {
						v1 = getVertexIndex(i, j - 1, k - 1);
						v2 = getVertexIndex(i, j, k - 1);
						v3 = getVertexIndex(i, j - 1, k);
						v4 = getVertexIndex(i, j, k);

						pushRectangle(indices, v1, v2, v3, v4);
					}

					if (blocks[i][j - 1][k] == 0) {
						v1 = getVertexIndex(i - 1, j - 1, k - 1);
						v2 = getVertexIndex(i, j - 1, k - 1);
						v3 = getVertexIndex(i - 1, j - 1, k);
						v4 = getVertexIndex(i, j - 1, k);

						pushRectangle(indices, v1, v2, v3, v4);
					}

					if (blocks[i][j + 1][k] == 0) {
						v1 = getVertexIndex(i - 1, j, k - 1);
						v2 = getVertexIndex(i - 1, j, k);
						v3 = getVertexIndex(i, j, k - 1);
						v4 = getVertexIndex(i, j, k);

						pushRectangle(indices, v1, v2, v3, v4);
					}

					if (blocks[i][j][k - 1] == 0) {
						v1 = getVertexIndex(i - 1, j - 1, k - 1);
						v2 = getVertexIndex(i - 1, j, k - 1);
						v3 = getVertexIndex(i, j - 1, k - 1);
						v4 = getVertexIndex(i, j, k - 1);

						pushRectangle(indices, v1, v2, v3, v4);
					}

					if (blocks[i][j][k + 1] == 0) {
						v1 = getVertexIndex(i - 1, j - 1, k);
						v2 = getVertexIndex(i, j - 1, k);
						v3 = getVertexIndex(i - 1, j, k);
						v4 = getVertexIndex(i, j, k);

						pushRectangle(indices, v1, v2, v3, v4);
					}
				}
			}
		}
	}

	return indices;
}

void ChunkGenerator::updateMeshed(
	bool meshed[][CHUNK_SIZE],
	const uint32_t secondaryDimension,
	const uint32_t tertiaryDimension,
	const uint32_t dim1,
	const uint32_t dim2) const {

	for (uint32_t j = secondaryDimension - 1; j < dim2 - 1; ++j) {
		for (uint32_t k = tertiaryDimension - 1; k < dim1 - 1; ++k) {
			meshed[j][k] = true;
		}
	}
}

std::vector<uint32_t> ChunkGenerator::generateXSlice(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE], int32_t side) const {
	std::vector<uint32_t> slices;
	for (uint32_t i = 1; i < CHUNK_SIZE + 1; ++i) {
		bool meshed[CHUNK_SIZE][CHUNK_SIZE] = {};
		for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
			for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
				if (meshed[j - 1][k - 1] == false
					&& blocks[i][j][k] == true
					&& blocks[i + side][j][k] == false) {

					uint32_t dim1 = k - 1;
					do {
						++dim1;
					} while (meshed[j - 1][dim1 - 1] == false
						&& blocks[i][j][dim1] == true
						&& blocks[i + side][j][dim1] == false
						&& dim1 < CHUNK_SIZE + 1);

					uint32_t dim2 = j + 1;
					do {
						bool holeOrObstructionFound = false;
						for (uint32_t m = k; m < dim1; ++m) {
							if (blocks[i][dim2][m] == false
								|| blocks[i + side][dim2][m] == true) {
								holeOrObstructionFound = true;
								break;
							}
						}

						if (holeOrObstructionFound) {
							break;
						}

						++dim2;
					} while (dim2 < CHUNK_SIZE + 1);

					updateMeshed(meshed, j, k, dim1, dim2);

					if (side == -1) {
						uint32_t v1 = getVertexIndex(i - 1, k - 1, j - 1);
						uint32_t v2 = getVertexIndex(i - 1, k - 1, dim2 - 1);
						uint32_t v3 = getVertexIndex(i - 1, dim1 - 1, j - 1);
						uint32_t v4 = getVertexIndex(i - 1, dim1 - 1, dim2 - 1);

						pushRectangle(slices, v1, v2, v3, v4);
					}
					else {
						uint32_t v1 = getVertexIndex(i, k - 1, j - 1);
						uint32_t v2 = getVertexIndex(i, dim1 - 1, j - 1);
						uint32_t v3 = getVertexIndex(i, k - 1, dim2 - 1);
						uint32_t v4 = getVertexIndex(i, dim1 - 1, dim2 - 1);

						pushRectangle(slices, v1, v2, v3, v4);
					}
				}
			}
		}
	}
	return slices;
}

std::vector<uint32_t> ChunkGenerator::generateYSlice(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE], int32_t side) const {
	std::vector<uint32_t> slices;
	for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
		bool meshed[CHUNK_SIZE][CHUNK_SIZE] = {};
		for (uint32_t i = 1; i < CHUNK_SIZE + 1; ++i) {
			for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
				if (meshed[i - 1][k - 1] == false
					&& blocks[i][j][k] == true
					&& blocks[i][j + side][k] == false) {

					uint32_t dim1 = k - 1;
					do {
						++dim1;
					} while (meshed[i - 1][dim1 - 1] == false
						&& blocks[i][j][dim1] == true
						&& blocks[i][j + side][dim1] == false
						&& dim1 < CHUNK_SIZE + 1);

					uint32_t dim2 = i + 1;
					do {
						bool holeOrObstructionFound = false;
						for (uint32_t m = k; m < dim1; ++m) {
							if (blocks[dim2][j][m] == false
								|| blocks[dim2][j + side][m] == true) {
								holeOrObstructionFound = true;
								break;
							}
						}

						if (holeOrObstructionFound) {
							break;
						}

						++dim2;
					} while (dim2 < CHUNK_SIZE + 1);


					updateMeshed(meshed, i, k, dim1, dim2);

					if (side == -1) {
						uint32_t v1 = getVertexIndex(i - 1, k - 1, j - 1);
						uint32_t v2 = getVertexIndex(i - 1, dim1 - 1, j - 1);
						uint32_t v3 = getVertexIndex(dim2 - 1, k - 1, j - 1);
						uint32_t v4 = getVertexIndex(dim2 - 1, dim1 - 1, j - 1);

						pushRectangle(slices, v1, v2, v3, v4);
					}
					else {
						uint32_t v1 = getVertexIndex(i - 1, k - 1, j);
						uint32_t v2 = getVertexIndex(dim2 - 1, k - 1, j);
						uint32_t v3 = getVertexIndex(i - 1, dim1 - 1, j);
						uint32_t v4 = getVertexIndex(dim2 - 1, dim1 - 1, j);

						pushRectangle(slices, v1, v2, v3, v4);
					}
				}
			}
		}
	}
	return slices;
}

std::vector<uint32_t> ChunkGenerator::generateZSlice(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE], int32_t side) const {
	std::vector<uint32_t> slices;
	for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
		bool meshed[CHUNK_SIZE][CHUNK_SIZE] = {};
		for (uint32_t i = 1; i < CHUNK_SIZE + 1; ++i) {
			for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
				if (meshed[i - 1][j - 1] == false
					&& blocks[i][j][k] == true
					&& blocks[i][j][k + side] == false) {

					uint32_t dim1 = j - 1;
					do {
						++dim1;
					} while (meshed[i - 1][dim1 - 1] == false
						&& blocks[i][dim1][k] == true
						&& blocks[i][dim1][k + side] == false
						&& dim1 < CHUNK_SIZE + 1);

					uint32_t dim2 = i + 1;
					do {
						bool holeOrObstructionFound = false;
						for (uint32_t m = j; m < dim1; ++m) {
							if (blocks[dim2][m][k] == false
								|| blocks[dim2][j][k + side] == true) {
								holeOrObstructionFound = true;
								break;
							}
						}

						if (holeOrObstructionFound) {
							break;
						}

						++dim2;
					} while (dim2 < CHUNK_SIZE + 1);

					updateMeshed(meshed, i, j, dim1, dim2);

					if (side == -1) {
						uint32_t v1 = getVertexIndex(i - 1, k - 1, j - 1);
						uint32_t v2 = getVertexIndex(dim2 - 1, k - 1, j - 1);
						uint32_t v3 = getVertexIndex(i - 1, k - 1, dim1 - 1);
						uint32_t v4 = getVertexIndex(dim2 - 1, k - 1, dim1 - 1);

						pushRectangle(slices, v1, v2, v3, v4);
					}
					else {
						uint32_t v1 = getVertexIndex(i - 1, k, j - 1);
						uint32_t v2 = getVertexIndex(i - 1, k, dim1 - 1);
						uint32_t v3 = getVertexIndex(dim2 - 1, k, j - 1);
						uint32_t v4 = getVertexIndex(dim2 - 1, k, dim1 - 1);

						pushRectangle(slices, v1, v2, v3, v4);
					}
				}
			}
		}
	}
	return slices;
}

std::vector<uint32_t> ChunkGenerator::combineSlices(const std::vector<std::vector<uint32_t>>& slices) const {
	std::vector<uint32_t> indices;

	std::size_t totalSize = 0;
	for (const auto& slice : slices) {
		totalSize += slice.size();
	}

	indices.reserve(totalSize);
	for (const auto& slice : slices) {
		indices.insert(indices.end(), slice.begin(), slice.end());
	}

	return indices;
}

std::vector<uint32_t> ChunkGenerator::generateGreedyTriangles(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE]) const {
	std::vector<std::vector<uint32_t>> slices;
	slices.push_back(generateXSlice(blocks, -1));
	slices.push_back(generateXSlice(blocks, 1));
	slices.push_back(generateYSlice(blocks, -1));
	slices.push_back(generateYSlice(blocks, 1));
	slices.push_back(generateZSlice(blocks, -1));
	slices.push_back(generateZSlice(blocks, 1));

	return combineSlices(slices);
}

std::vector<uint32_t> ChunkGenerator::generateGreedyTrianglesMultithreaded(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE]) const {
	std::vector<std::future<std::vector<uint32_t>>> futures;

	futures.push_back(std::async(std::launch::async, &ChunkGenerator::generateXSlice, this, blocks, -1));
	futures.push_back(std::async(std::launch::async, &ChunkGenerator::generateXSlice, this, blocks, 1));
	futures.push_back(std::async(std::launch::async, &ChunkGenerator::generateYSlice, this, blocks, -1));
	futures.push_back(std::async(std::launch::async, &ChunkGenerator::generateYSlice, this, blocks, 1));
	futures.push_back(std::async(std::launch::async, &ChunkGenerator::generateZSlice, this, blocks, -1));
	futures.push_back(std::async(std::launch::async, &ChunkGenerator::generateZSlice, this, blocks, 1));

	std::vector<std::vector<uint32_t>> slices;
	slices.reserve(VERTEX_SIZE_CUBED);
	for (auto& future : futures) {
		slices.push_back(future.get());
	}

	return combineSlices(slices);
}
