#include "taskflow/priority_strategy.hpp"

namespace taskflow {

int DefaultPriorityStrategy::score(const Task& task) const {
    return 10 - task.priority;
}

int ClosedLastPriorityStrategy::score(const Task& task) const {
    if (is_closed(task.status)) {
        return 1000;
    }
    return 10 - task.priority;
}

int DueDatePriorityStrategy::score(const Task& task) const {
    if (is_closed(task.status)) {
        return 2000;
    }
    // меньше дней до дедлайна → меньше score → выше в списке
    return task.days_left;
}

} // namespace taskflow
