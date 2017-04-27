#include "chunk.h"

float NYChunk::_WorldVert[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE * 3 * 3 * 6 * 2];
float NYChunk::_WorldCols[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE * 3 * 3 * 6 * 2];
float NYChunk::_WorldNorm[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE * 3 * 3 * 6 * 2];