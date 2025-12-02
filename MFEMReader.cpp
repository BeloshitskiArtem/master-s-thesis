#include "stdafx.h"
#include "HeatSolver.cpp"

/*
* @brief Читатель файла .MESH
*/
class MFEMReader {
public:
	bool readFile(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Cannot open file: " << filename << std::endl;
			return false;
		}

		std::string line;
		int state = 0;

		while (std::getline(file, line))
		{
			if (line.empty() || line[0] == '#') continue;

			if (!line.empty() && line.back() == '\r') {
				line.pop_back();
			}

			if (line == "dimension")
			{
				state = 1;
			}
			else if (line == "elements")
			{
				state = 2;
			}
			else if (line == "boundary")
			{
				state = 3;
			}
			else if (line == "vertices")
			{
				state = 4;
			}
			else
			{
				std::istringstream iss(line);

				switch (state)
				{
				case 1: // dimension
					iss >> dimension;
					break;
				case 2: // elements
					if (num_elements == 0)
					{
						iss >> num_elements;
					}
					else
					{
						readElement(line);
					}
					break;
				case 3: // boundary
					if (num_boundary == 0)
					{
						iss >> num_boundary;
					}
					break;
				case 4: // vertices
					if (num_vertices == 0)
					{
						iss >> num_vertices;
						std::getline(file, line);
					}
					else
					{
						readVertex(line);
					}
					break;
				}
			}
		}

		file.close();
		extractEdgesFromElements();
		return true;
	}

	std::vector<Vertex> getVertices() const { return vertices; }
	std::vector<Edge> getEdges() const { return edges; }

	void printMeshInfo() const {
		std::cout << "Mesh information:" << std::endl;
		std::cout << "Vertices: " << vertices.size() << std::endl;
		for (size_t i = 0; i < vertices.size(); i++) {
			std::cout << "  V" << i << ": (" << vertices[i].x << ", "
				<< vertices[i].y << ", " << vertices[i].z << ")" << std::endl;
		}
		std::cout << "Edges: " << edges.size() << std::endl;
		for (size_t i = 0; i < edges.size(); i++) {
			std::cout << "  E" << i << ": V" << edges[i].start_index << " -> V"
				<< edges[i].end_index << " (length: " << edges[i].length << ")" << std::endl;
		}
	}

private:
	int dimension = 0;
	int num_elements = 0;
	int num_boundary = 0;
	int num_vertices = 0;
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;

	struct Element {
		int type;
		std::vector<int> vertex_indices;
	};
	std::vector<Element> elements;

	void readElement(const std::string& line) {
		std::istringstream iss(line);
		Element element;
		int element_id;

		iss >> element_id >> element.type;

		int vertex_idx;
		while (iss >> vertex_idx) {
			element.vertex_indices.push_back(vertex_idx);
		}

		elements.push_back(element);
	}

	void readVertex(const std::string& line) {
		std::istringstream iss(line);
		Vertex vertex;

		if (dimension == 3) {
			iss >> vertex.x >> vertex.y >> vertex.z;
		}
		else if (dimension == 2) {
			iss >> vertex.x >> vertex.y;
			vertex.z = 0.0;
		}

		vertices.push_back(vertex);
	}

	void extractEdgesFromElements() {
		std::set<std::pair<int, int>> unique_edges;

		for (const auto& element : elements) {
			const auto& indices = element.vertex_indices;

			// Для каждого типа элемента определяем его рёбра
			if (element.type == 5 && indices.size() == 8) { // CUBE
				addCubeEdges(unique_edges, indices);
			}
			else if (element.type == 4 && indices.size() == 4) { // TETRAHEDRON
				addTetrahedronEdges(unique_edges, indices);
			}
			else if (element.type == 2 && indices.size() == 3) { // TRIANGLE
				addTriangleEdges(unique_edges, indices);
			}
			// Можно добавить другие типы элементов
		}

		// Создаём объекты Edge
		for (const auto& edge_pair : unique_edges) {
			Edge edge;
			edge.start_index = edge_pair.first;
			edge.end_index = edge_pair.second;

			const Vertex& v1 = vertices[edge.start_index];
			const Vertex& v2 = vertices[edge.end_index];
			double dx = v2.x - v1.x;
			double dy = v2.y - v1.y;
			double dz = v2.z - v1.z;
			edge.length = std::sqrt(dx*dx + dy*dy + dz*dz);

			edges.push_back(edge);
		}
	}

	void addCubeEdges(std::set<std::pair<int, int>>& edges, const std::vector<int>& indices) {
		// 12 рёбер куба
		addEdge(edges, indices[0], indices[1]);
		addEdge(edges, indices[1], indices[2]);
		addEdge(edges, indices[2], indices[3]);
		addEdge(edges, indices[3], indices[0]);

		addEdge(edges, indices[4], indices[5]);
		addEdge(edges, indices[5], indices[6]);
		addEdge(edges, indices[6], indices[7]);
		addEdge(edges, indices[7], indices[4]);

		addEdge(edges, indices[0], indices[4]);
		addEdge(edges, indices[1], indices[5]);
		addEdge(edges, indices[2], indices[6]);
		addEdge(edges, indices[3], indices[7]);
	}

	void addTetrahedronEdges(std::set<std::pair<int, int>>& edges, const std::vector<int>& indices) {
		// 6 рёбер тетраэдра
		addEdge(edges, indices[0], indices[1]);
		addEdge(edges, indices[0], indices[2]);
		addEdge(edges, indices[0], indices[3]);
		addEdge(edges, indices[1], indices[2]);
		addEdge(edges, indices[1], indices[3]);
		addEdge(edges, indices[2], indices[3]);
	}

	void addTriangleEdges(std::set<std::pair<int, int>>& edges, const std::vector<int>& indices) {
		// 3 ребра треугольника
		addEdge(edges, indices[0], indices[1]);
		addEdge(edges, indices[1], indices[2]);
		addEdge(edges, indices[2], indices[0]);
	}

	void addEdge(std::set<std::pair<int, int>>& edges, int v1, int v2) {
		if (v1 < v2) {
			edges.insert({ v1, v2 });
		}
		else {
			edges.insert({ v2, v1 });
		}
	}
};
