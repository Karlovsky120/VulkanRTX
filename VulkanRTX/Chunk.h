#pragma once

#include "Vertex.h"

#include <glm/vec3.hpp>

#include <chrono>
#include <iostream>
#include <future>
#include <stdint.h>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <queue>

#define CHUNK_SIZE 32
#define CHUNK_SIZE_SQUARED (CHUNK_SIZE*CHUNK_SIZE)

#define VERTEX_SIZE (CHUNK_SIZE+1)
#define VERTEX_SIZE_SQUARED (VERTEX_SIZE*VERTEX_SIZE)
#define VERTEX_SIZE_CUBED (VERTEX_SIZE_SQUARED*VERTEX_SIZE)

#define CHUNK_PADDED_SIZE (VERTEX_SIZE+1)
#define CHUNK_PADDED_SIZE_SQUARED (CHUNK_PADDED_SIZE*CHUNK_PADDED_SIZE)
#define CHUNK_PADDED_SIZE_CUBED (CHUNK_PADDED_SIZE_SQUARED*CHUNK_PADDED_SIZE)

class Chunk {
	bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE] = {};

public:
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	void generateRandomly() {
		srand(1);

		for (uint32_t i = 1; i < CHUNK_SIZE + 1; ++i) {
			for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
				for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
					blocks[i][j][k] = rand() % 2 == 0;
				}
			}
		}

		/*for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
			for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
				blocks[1][j][k] = rand() % 3 == 0;
			}
		}*/

		//blocks[getBlockIndex(1, 2, 2)] = false;
	}

	void generateVertices() {
		for (uint32_t i = 0; i < VERTEX_SIZE; ++i) {
			for (uint32_t j = 0; j < VERTEX_SIZE; ++j) {
				for (uint32_t k = 0; k < VERTEX_SIZE; ++k) {
					vertices.push_back({
						glm::vec3(
							i - static_cast<float>(CHUNK_SIZE) * 0.5,
							j - static_cast<float>(CHUNK_SIZE) * 0.5,
							k - static_cast<float>(CHUNK_SIZE) * 0.5),
						{}
					});
				}
			}
		}
	}

	inline uint32_t getBlockIndex(uint32_t x, uint32_t y, uint32_t z) const {
		return x * CHUNK_PADDED_SIZE_SQUARED + y * CHUNK_PADDED_SIZE + z;
	}

	inline uint32_t getVertexIndex(uint32_t x, uint32_t y, uint32_t z) const {
		return x * VERTEX_SIZE_SQUARED + y * VERTEX_SIZE + z;
	}

	inline void pushRectangle(std::vector<uint16_t>& v, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4) {
		v.push_back(v1);
		v.push_back(v2);
		v.push_back(v3);
		v.push_back(v2);
		v.push_back(v4);
		v.push_back(v3);
	}

	void generateSimpleTriangles() {
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
	}

	/*void generateSimpleTriangles() {
		for (uint32_t i = 1; i < CHUNK_SIZE + 1; ++i) {
			for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
				for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
					if (blocks[getBlockIndex(i, j, k)] == 1) {
						uint32_t v1;
						uint32_t v3;
						uint32_t v2;
						uint32_t v4;

						if (blocks[getBlockIndex(i - 1, j, k)] == 0) {
							v1 = getVertexIndex(i - 1, j - 1, k - 1);
							v3 = getVertexIndex(i - 1, j, k - 1);
							v2 = getVertexIndex(i - 1, j - 1, k);
							v4 = getVertexIndex(i - 1, j, k);

							pushRectangle(v1, v2, v3, v4);
						}

						if (blocks[getBlockIndex(i + 1, j, k)] == 0) {
							v1 = getVertexIndex(i, j - 1, k - 1);
							v2 = getVertexIndex(i, j, k - 1);
							v3 = getVertexIndex(i, j - 1, k);
							v4 = getVertexIndex(i, j, k);

							pushRectangle(v1, v2, v3, v4);
						}

						if (blocks[getBlockIndex(i, j - 1, k)] == 0) {
							v1 = getVertexIndex(i - 1, j - 1, k - 1);
							v2 = getVertexIndex(i, j - 1, k - 1);
							v3 = getVertexIndex(i - 1, j - 1, k);
							v4 = getVertexIndex(i, j - 1, k);

							pushRectangle(v1, v2, v3, v4);
						}

						if (blocks[getBlockIndex(i, j + 1, k)] == 0) {
							v1 = getVertexIndex(i - 1, j, k - 1);
							v2 = getVertexIndex(i - 1, j, k);
							v3 = getVertexIndex(i, j, k - 1);
							v4 = getVertexIndex(i, j, k);

							pushRectangle(v1, v2, v3, v4);
						}

						if (blocks[getBlockIndex(i, j, k - 1)] == 0) {
							v1 = getVertexIndex(i - 1, j - 1, k - 1);
							v2 = getVertexIndex(i - 1, j, k - 1);
							v3 = getVertexIndex(i, j - 1, k - 1);
							v4 = getVertexIndex(i, j, k - 1);

							pushRectangle(v1, v2, v3, v4);
						}

						if (blocks[getBlockIndex(i, j, k + 1)] == 0) {
							v1 = getVertexIndex(i - 1, j - 1, k);
							v2 = getVertexIndex(i, j - 1, k);
							v3 = getVertexIndex(i - 1, j, k);
							v4 = getVertexIndex(i, j, k);

							pushRectangle(v1, v2, v3, v4);
						}
					}
				}
			}
		}
	}

	void generateNaiveTriangles() {
		for (uint32_t i = 1; i < CHUNK_SIZE + 1; ++i) {
			for (uint32_t j = 1; j < CHUNK_SIZE + 1; ++j) {
				for (uint32_t k = 1; k < CHUNK_SIZE + 1; ++k) {
					if (blocks[getBlockIndex(i, j, k)] == 1) {
						uint32_t v1;
						uint32_t v3;
						uint32_t v2;
						uint32_t v4;

						v1 = getVertexIndex(i - 1, j - 1, k - 1);
						v3 = getVertexIndex(i - 1, j, k - 1);
						v2 = getVertexIndex(i - 1, j - 1, k);
						v4 = getVertexIndex(i - 1, j, k);

						pushRectangle(v1, v2, v3, v4);

						v1 = getVertexIndex(i, j - 1, k - 1);
						v2 = getVertexIndex(i, j, k - 1);
						v3 = getVertexIndex(i, j - 1, k);
						v4 = getVertexIndex(i, j, k);

						pushRectangle(v1, v2, v3, v4);

						v1 = getVertexIndex(i - 1, j - 1, k - 1);
						v2 = getVertexIndex(i, j - 1, k - 1);
						v3 = getVertexIndex(i - 1, j - 1, k);
						v4 = getVertexIndex(i, j - 1, k);

						pushRectangle(v1, v2, v3, v4);

						v1 = getVertexIndex(i - 1, j, k - 1);
						v2 = getVertexIndex(i - 1, j, k);
						v3 = getVertexIndex(i, j, k - 1);
						v4 = getVertexIndex(i, j, k);

						pushRectangle(v1, v2, v3, v4);

						v1 = getVertexIndex(i - 1, j - 1, k - 1);
						v2 = getVertexIndex(i - 1, j, k - 1);
						v3 = getVertexIndex(i, j - 1, k - 1);
						v4 = getVertexIndex(i, j, k - 1);

						pushRectangle(v1, v2, v3, v4);

						v1 = getVertexIndex(i - 1, j - 1, k);
						v2 = getVertexIndex(i, j - 1, k);
						v3 = getVertexIndex(i - 1, j, k);
						v4 = getVertexIndex(i, j, k);

						pushRectangle(v1, v2, v3, v4);
					}
				}
			}
		}
	}*/

	void updateMeshed(
		bool meshed[][CHUNK_SIZE],
		const uint32_t secondaryDimension,
		const uint32_t tertiaryDimension,
		const uint32_t dim1,
		const uint32_t dim2) {

		for (uint32_t j = secondaryDimension - 1; j < dim2 - 1; ++j) {
			for (uint32_t k = tertiaryDimension - 1; k < dim1 - 1; ++k) {
				meshed[j][k] = true;
			}
		}
	}

	std::vector<uint16_t> generateXSlice(int32_t side) {
		std::vector<uint16_t> slices;
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

	std::vector<uint16_t> generateYSlice(int32_t side) {
		std::vector<uint16_t> slices;
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

	std::vector<uint16_t> generateZSlice(int32_t side) {
		std::vector<uint16_t> slices;
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

	void combineSlices(std::vector<std::vector<uint16_t>>& slices) {
		std::size_t totalSize = 0;
		for (const auto& slice : slices) {
			totalSize += slice.size();
		}

		indices.reserve(totalSize);
		for (const auto& slice : slices) {
			indices.insert(indices.end(), slice.begin(), slice.end());
		}
	}

	void generateGreedyTriangles() {
		std::vector<std::vector<uint16_t>> slices;

		long long totalTime = 0;

		//for (uint32_t k = 0; k < 1000; ++k) {
			auto start = std::chrono::high_resolution_clock::now();

			slices = {};
			slices.push_back(generateXSlice(-1));
			slices.push_back(generateXSlice(1));
			slices.push_back(generateYSlice(-1));
			slices.push_back(generateYSlice(1));
			slices.push_back(generateZSlice(-1));
			slices.push_back(generateZSlice(1));
			
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			std::cout << duration << std::endl;
			totalTime += duration;
		//}

		std::cout << totalTime / 1000 << std::endl;

		combineSlices(slices);
	}

	void generateGreedyTrianglesMultithreaded() {
		std::vector<std::vector<uint16_t>> slices;

		long long totalTime = 0;

		//for (uint32_t k = 0; k < 1000; ++k) {
			auto start = std::chrono::high_resolution_clock::now();

			std::vector<std::future<std::vector<uint16_t>>> futures;

			futures.push_back(std::async(std::launch::async, &Chunk::generateXSlice, this, -1));
			futures.push_back(std::async(std::launch::async, &Chunk::generateXSlice, this, 1));
			futures.push_back(std::async(std::launch::async, &Chunk::generateYSlice, this, -1));
			futures.push_back(std::async(std::launch::async, &Chunk::generateYSlice, this, 1));
			futures.push_back(std::async(std::launch::async, &Chunk::generateZSlice, this, -1));
			futures.push_back(std::async(std::launch::async, &Chunk::generateZSlice, this, 1));

			slices = {};
			slices.reserve(VERTEX_SIZE_CUBED);
			for (auto& future : futures) {
				slices.push_back(future.get());
			}

			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			std::cout << duration << std::endl;
			totalTime += duration;
		//}

		std::cout << totalTime / 1000 << std::endl;

		combineSlices(slices);
	}
};
