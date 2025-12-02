#include "stdafx.h"

/*
* @brief ¬ершина триангул€ции.
*/
struct Vertex
{
	double x, y, z;
};

/*
* @brief –ебро триангул€ции.
*/
struct Edge
{
	int start_index;    // »ндекс начальной вершины
	int end_index;      // »ндекс конечной вершины
	double length;      // ƒлина ребра
};