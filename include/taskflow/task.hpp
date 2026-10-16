#pragma once
#include <cstdint>
#include <string>

namespace taskflow {

enum class TaskStatus { New, InProgress, Done, Cancelled };

struct Task {
    std::int64_t id{};
    std::string title;
    std::string assignee;
    TaskStatus status{TaskStatus::New};
    int priority{3};
    std::string description; // ЛР1
    int days_left{7};        // ЛР2: сколько дней до дедлайна (для стратегии)
};

inline bool is_closed(TaskStatus status) {
    return status == TaskStatus::Done || status == TaskStatus::Cancelled;
}

std::string to_string(TaskStatus status);
TaskStatus status_from_string(const std::string& value);

} // namespace taskflow
