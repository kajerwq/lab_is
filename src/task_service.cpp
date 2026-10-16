#include "taskflow/task_service.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace taskflow {

std::string to_string(TaskStatus status) {
    switch (status) {
        case TaskStatus::New: return "New";
        case TaskStatus::InProgress: return "InProgress";
        case TaskStatus::Done: return "Done";
        case TaskStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

TaskStatus status_from_string(const std::string& value) {
    if (value == "New") return TaskStatus::New;
    if (value == "InProgress") return TaskStatus::InProgress;
    if (value == "Done") return TaskStatus::Done;
    if (value == "Cancelled") return TaskStatus::Cancelled;
    throw std::invalid_argument("bad status: " + value);
}

TaskService::TaskService(TaskRepositoryPtr repository, PriorityStrategyPtr priority_strategy)
    : repo_(std::move(repository)), strategy_(std::move(priority_strategy)) {
    if (!repo_ || !strategy_) {
        throw std::invalid_argument("repo/strategy required");
    }
}

Task TaskService::create_task(std::string title, std::string assignee, int priority,
                              std::string description, int days_left) {
    check_input(title, assignee, priority, description);
    Task draft;
    draft.id = 0;
    draft.title = std::move(title);
    draft.assignee = std::move(assignee);
    draft.status = TaskStatus::New;
    draft.priority = priority;
    draft.description = std::move(description);
    draft.days_left = days_left;
    return repo_->create(std::move(draft));
}

bool TaskService::assign(std::int64_t id, const std::string& new_assignee) {
    if (new_assignee.empty()) {
        throw std::invalid_argument("assignee is empty");
    }
    auto task = repo_->find_by_id(id);
    if (!task) {
        return false;
    }
    // нельзя назначать закрытую задачу
    if (is_closed(task->status)) {
        return false;
    }
    task->assignee = new_assignee;
    return repo_->update(*task);
}

bool TaskService::transition(std::int64_t id, TaskStatus next_status) {
    auto task = repo_->find_by_id(id);
    if (!task) {
        return false;
    }
    if (!allowed_transition(task->status, next_status)) {
        return false;
    }
    task->status = next_status;
    return repo_->update(*task);
}

std::vector<Task> TaskService::list_by_priority() const {
    auto tasks = repo_->list();
    std::sort(tasks.begin(), tasks.end(), [&](const Task& a, const Task& b) {
        const int sa = strategy_->score(a);
        const int sb = strategy_->score(b);
        if (sa == sb) {
            return a.id < b.id;
        }
        return sa < sb;
    });
    return tasks;
}

std::optional<Task> TaskService::find(std::int64_t id) const {
    return repo_->find_by_id(id);
}

void TaskService::check_input(const std::string& title, const std::string& assignee,
                              int priority, const std::string& description) const {
    if (title.empty()) {
        throw std::invalid_argument("title is empty");
    }
    if (assignee.empty()) {
        throw std::invalid_argument("assignee is empty");
    }
    if (priority < 1 || priority > 5) {
        throw std::invalid_argument("priority out of range");
    }
    if (description.size() > 1000) {
        throw std::invalid_argument("description too long");
    }
}

bool TaskService::allowed_transition(TaskStatus from, TaskStatus to) const {
    if (from == to) {
        return true;
    }
    if (is_closed(from)) {
        return false;
    }
    if (from == TaskStatus::New) {
        return to == TaskStatus::InProgress || to == TaskStatus::Cancelled;
    }
    if (from == TaskStatus::InProgress) {
        return to == TaskStatus::Done || to == TaskStatus::Cancelled;
    }
    return false;
}

} // namespace taskflow
