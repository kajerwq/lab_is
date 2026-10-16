#pragma once
#include "taskflow/priority_strategy.hpp"
#include "taskflow/repository.hpp"
#include <string>
#include <vector>

namespace taskflow {

class TaskService {
public:
    TaskService(TaskRepositoryPtr repository, PriorityStrategyPtr priority_strategy);

    Task create_task(std::string title, std::string assignee, int priority,
                     std::string description = "", int days_left = 7);
    bool assign(std::int64_t id, const std::string& new_assignee);
    bool transition(std::int64_t id, TaskStatus next_status);
    std::vector<Task> list_by_priority() const;
    std::optional<Task> find(std::int64_t id) const;

private:
    void check_input(const std::string& title, const std::string& assignee,
                     int priority, const std::string& description) const;
    bool allowed_transition(TaskStatus from, TaskStatus to) const;

    TaskRepositoryPtr repo_;
    PriorityStrategyPtr strategy_;
};

} // namespace taskflow
