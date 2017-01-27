#include "chunk.h"

float NYChunk::_WorldVert[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE*3*4*6];
float NYChunk:: _WorldCols[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE*3*4*6];
float NYChunk::_WorldNorm[X_CHUNK_SIZE*Y_CHUNK_SIZE*Z_CHUNK_SIZE*3*4*6];