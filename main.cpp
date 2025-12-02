// testus.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "MFEMReader.cpp"

/*
* @brief Точа входа.
*/
int main() {
	const std::string INPUT_FILE = "C:\\Users\\5801bap\\Desktop\\xz_test.mesh"; /*"C:\\Users\\5801bap\\Desktop\\mfem-4.5.2\\data\\cube-nurbs.mesh";*/
	const std::string MESH_OUTPUT = /*"C:\\Users\\Artem\\Desktop\\mesh_data.txt";*/ "C:\\Users\\5801bap\\Desktop\\mesh_data.txt";
	const std::string HEAT_OUTPUT = /*"C:\\Users\\Artem\\Desktop\\heat_results.csv";*/"C:\\Users\\5801bap\\Desktop\\heat_results.csv";

	// Параметры расчета
	const int HOT_VERTEX = 0;           // Вершина с приложенной температурой
	const double HOT_TEMPERATURE = 100.0; // Температура нагрева
	const double INITIAL_TEMP = 21.0;   // Начальная температура всех точек
	const double TIME_STEP = 0.1;       // Шаг по времени (сек)
	const double MEASUREMENT_INTERVAL = 1.0; // Интервал измерения (сек)
	const double TOTAL_TIME = 10.0;     // Общее время расчета (сек)
	const double THERMAL_DIFFUSIVITY = 0.05; // Коэффициент температуропроводности

	// Чтение сетки
	MFEMReader reader;
	if (!reader.readFile(INPUT_FILE)) {
		std::cerr << "Failed to read input file: " << INPUT_FILE << std::endl;
		return 1;
	}

	auto vertices = reader.getVertices();
	auto edges = reader.getEdges();

	reader.printMeshInfo();

	// Создаем и настраиваем решатель
	HeatSolver solver(THERMAL_DIFFUSIVITY, TIME_STEP, INITIAL_TEMP);
	solver.setMesh(vertices, edges);
	solver.setTemperature(HOT_VERTEX, HOT_TEMPERATURE);

	std::cout << "\nSimulation parameters:" << std::endl;
	std::cout << "Heated vertex: V" << HOT_VERTEX << " at " << HOT_TEMPERATURE << "°C" << std::endl;
	std::cout << "Initial temperature: " << INITIAL_TEMP << "°C" << std::endl;
	std::cout << "Thermal diffusivity: " << THERMAL_DIFFUSIVITY << std::endl;

	// Запускаем расчет
	solver.solveForTime(TOTAL_TIME, MEASUREMENT_INTERVAL);
	// Сохраняем результаты
	solver.saveResults(HEAT_OUTPUT);
	std::cout << "\nSimulation completed!" << std::endl;

	return 0;
}