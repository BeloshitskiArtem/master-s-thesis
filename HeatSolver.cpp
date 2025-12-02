#include "stdafx.h"
#include "Structures.cpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <cmath>
#include <iomanip>

class HeatSolver
{
private:
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;
	std::vector<double> temperature;
	std::vector<double> new_temperature;
	double thermal_diffusivity;
	double time_step;
	double initial_temp;

public:
	HeatSolver(double alpha = 0.1, double dt = 0.01, double init_temp = 21.0)
		: thermal_diffusivity(alpha), time_step(dt), initial_temp(init_temp) {}

	void setMesh(const std::vector<Vertex>& verts, const std::vector<Edge>& edgs) {
		vertices = verts;
		edges = edgs;
		temperature.resize(vertices.size(), initial_temp);
		new_temperature.resize(vertices.size(), initial_temp);
	}

	// Установка температуры в конкретной вершине
	void setTemperature(int vertex_index, double temp) {
		if (vertex_index >= 0 && vertex_index < temperature.size()) {
			temperature[vertex_index] = temp;
		}
	}

	// Получить текущее распределение температур
	const std::vector<double>& getTemperature() const {
		return temperature;
	}

	// Получить температуру в конкретной вершине
	double getTemperatureAt(int vertex_index) const {
		if (vertex_index >= 0 && vertex_index < temperature.size()) {
			return temperature[vertex_index];
		}
		return initial_temp;
	}

	// Найти все рёбра, связанные с вершиной
	std::vector<Edge> getEdgesForVertex(int vertex_index) const {
		std::vector<Edge> connected_edges;
		for (const auto& edge : edges) {
			if (edge.start_index == vertex_index || edge.end_index == vertex_index) {
				connected_edges.push_back(edge);
			}
		}
		return connected_edges;
	}

	// Один шаг решения уравнения теплопроводности
	void solveStep() {
		// Сохраняем граничные условия (температура в нагретой точке)
		double fixed_temperature = temperature[0]; // предполагаем, что вершина 0 нагрета

		for (size_t i = 0; i < vertices.size(); i++) {
			auto connected_edges = getEdgesForVertex(i);
			if (connected_edges.empty()) continue;

			double heat_flux = 0.0;
			int neighbor_count = 0;

			for (const auto& edge : connected_edges) {
				int neighbor_index = (edge.start_index == i) ? edge.end_index : edge.start_index;
				double temp_diff = temperature[neighbor_index] - temperature[i];
				double length = edge.length;

				if (length > 1e-10) {
					heat_flux += temp_diff / (length * length);
					neighbor_count++;
				}
			}

			if (neighbor_count > 0) {
				new_temperature[i] = temperature[i] + thermal_diffusivity * time_step * heat_flux;
			}
			else {
				new_temperature[i] = temperature[i];
			}
		}

		// Восстанавливаем граничное условие (температура в нагретой точке не меняется)
		new_temperature[0] = fixed_temperature;

		temperature = new_temperature;
	}

	// Решение на заданное время с выводом температур всех точек на каждом шаге
	void solveForTime(double total_time, double measurement_interval) {
		int total_steps = static_cast<int>(total_time / time_step);
		int measurement_steps = static_cast<int>(measurement_interval / time_step);

		if (measurement_steps == 0) measurement_steps = 1;

		std::cout << "=== Heat Transfer Simulation ===" << std::endl;
		std::cout << "Total simulation time: " << total_time << " seconds" << std::endl;
		std::cout << "Time step: " << time_step << " seconds" << std::endl;
		std::cout << "Measurement interval: " << measurement_interval << " seconds" << std::endl;
		std::cout << "Total steps: " << total_steps << std::endl;
		std::cout << "Vertices: " << vertices.size() << ", Edges: " << edges.size() << std::endl;
		std::cout << std::string(60, '=') << std::endl;

		// Вывод начального состояния
		std::cout << "Initial state (t = 0.0s):" << std::endl;
		printAllTemperatures();

		for (int step = 1; step <= total_steps; step++) {
			solveStep();

			double current_time = step * time_step;

			// Вывод через заданные интервалы измерения
			if (step % measurement_steps == 0 || step == total_steps) {
				std::cout << "\nTime: " << std::fixed << std::setprecision(2) << current_time << "s";
				std::cout << " (Step: " << step << "/" << total_steps << ")" << std::endl;
				printAllTemperatures();
			}

			// Проверка на выход за время измерения
			if (current_time >= total_time) {
				break;
			}
		}
	}

	// Вывод температур всех вершин
	void printAllTemperatures() const {
		std::cout << "Temperatures: ";
		for (size_t i = 0; i < temperature.size(); i++) {
			std::cout << "V" << i << ":" << std::fixed << std::setprecision(1) << temperature[i] << "°C";
			if (i < temperature.size() - 1) std::cout << ", ";
		}
		std::cout << std::endl;

		// Дополнительная информация
		//double min_temp = *std::min_element(temperature.begin(), temperature.end());
		//double max_temp = *std::max_element(temperature.begin(), temperature.end());
		//double avg_temp = std::accumulate(temperature.begin(), temperature.end(), 0.0) / temperature.size();

		//std::cout << "Summary: Min=" << min_temp << "°C, Max=" << max_temp
		//	<< "°C, Avg=" << avg_temp << "°C" << std::endl;
	}

	// Сохранение истории температур в файл
	void saveTemperatureHistory(const std::string& filename, const std::vector<std::vector<double>>& history,
		const std::vector<double>& time_points) const {
		std::ofstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Cannot create file: " << filename << std::endl;
			return;
		}

		// Заголовок CSV
		file << "Time";
		for (size_t i = 0; i < vertices.size(); i++) {
			file << ",V" << i;
		}
		file << std::endl;

		// Данные
		for (size_t t = 0; t < time_points.size(); t++) {
			file << std::fixed << std::setprecision(3) << time_points[t];
			for (size_t i = 0; i < vertices.size(); i++) {
				file << "," << std::fixed << std::setprecision(2) << history[t][i];
			}
			file << std::endl;
		}

		file.close();
		std::cout << "Temperature history saved to: " << filename << std::endl;
	}
	// Сохранение результатов в файл
	void saveResults(const std::string& filename) const {
		std::ofstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Cannot create file: " << filename << std::endl;
			return;
		}

		file << "vertex_index,x,y,z,temperature" << std::endl;
		for (size_t i = 0; i < vertices.size(); i++) {
			file << i << "," << vertices[i].x << "," << vertices[i].y << "," << vertices[i].z
				<< "," << temperature[i] << std::endl;
		}

		file.close();
		std::cout << "Results saved to: " << filename << std::endl;
	}
};
