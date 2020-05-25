#pragma once

#include "Vertex.h"

#include <vector>

#define CHUNK_SIZE 32
#define CHUNK_SIZE_SQUARED (CHUNK_SIZE*CHUNK_SIZE)

#define VERTEX_SIZE (CHUNK_SIZE+1)
#define VERTEX_SIZE_SQUARED (VERTEX_SIZE*VERTEX_SIZE)
#define VERTEX_SIZE_CUBED (VERTEX_SIZE_SQUARED*VERTEX_SIZE)

#define CHUNK_PADDED_SIZE (VERTEX_SIZE+1)
#define CHUNK_PADDED_SIZE_SQUARED (CHUNK_PADDED_SIZE*CHUNK_PADDED_SIZE)
#define CHUNK_PADDED_SIZE_CUBED (CHUNK_PADDED_SIZE_SQUARED*CHUNK_PADDED_SIZE)

class ChunkGenerator {
public:
	std::vector<Vertex> generateVertices();
	std::vector<uint32_t> generateChunk(uint32_t seed);

private:
	void generateRandomly(uint32_t seed, bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE]);

	const inline uint32_t getBlockIndex(uint32_t x, uint32_t y, uint32_t z) const;
	const inline uint32_t getVertexIndex(uint32_t x, uint32_t y, uint32_t z) const;

	const inline void pushRectangle(std::vector<uint32_t>& v, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)const;

	std::vector<uint32_t> generateSimpleTriangles(bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE]);

	void updateMeshed(
		bool meshed[][CHUNK_SIZE],
		const uint32_t secondaryDimension,
		const uint32_t tertiaryDimension,
		const uint32_t dim1,
		const uint32_t dim2) const;

	std::vector<uint32_t> generateXSlice(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE], int32_t side) const;
	std::vector<uint32_t> generateYSlice(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE], int32_t side) const;
	std::vector<uint32_t> generateZSlice(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE], int32_t side) const;
	std::vector<uint32_t> combineSlices(const std::vector<std::vector<uint32_t>>& slices) const;

	std::vector<uint32_t> generateGreedyTriangles(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE]) const;
	std::vector<uint32_t> generateGreedyTrianglesMultithreaded(const bool blocks[CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE][CHUNK_PADDED_SIZE]) const;
};