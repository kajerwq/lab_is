#pragma once
#include "taskflow/task.hpp"
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace taskflow {

// ЛР2 (индивидуальное): простой Adapter — разбор CSV-строки в Task
// Формат: title;assignee;priority;description
class CsvTaskAdapter {
public:
    static Task from_csv_line(const std::string& line) {
        Task task;
        std::stringstream ss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(ss, part, ';')) {
            parts.push_back(part);
        }
        if (parts.size() < 3) {
            throw std::invalid_argument("CSV line must have at least 3 fields");
        }
        task.title = parts[0];
        task.assignee = parts[1];
        task.priority = std::stoi(parts[2]);
        if (parts.size() >= 4) {
            task.description = parts[3];
        }
        task.status = TaskStatus::New;
        return task;
    }
};

} // namespace taskflow
