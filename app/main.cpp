#include "taskflow/csv_task_adapter.hpp"
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"
#include <iostream>
#include <memory>

int main() {
    using namespace taskflow;

    auto repo = std::make_shared<InMemoryTaskRepository>();
    auto strategy = std::make_shared<DueDatePriorityStrategy>();
    TaskService service(repo, strategy);

    // обычное создание
    service.create_task("Analyze requirements", "Anna", 2, "собрать ТЗ", 5);
    service.create_task("Code repository", "Boris", 1, "InMemory repo", 2);

    // ЛР2: импорт через CSV adapter
    auto from_csv = CsvTaskAdapter::from_csv_line("Write tests;Clara;3;unit tests");
    from_csv = repo->create(from_csv);

    std::cout << "TaskFlow (osipov) — tasks:\n";
    for (const auto& t : service.list_by_priority()) {
        std::cout << t.id << ") " << t.title << " / " << t.assignee
                  << " / " << to_string(t.status)
                  << " / p=" << t.priority
                  << " / days=" << t.days_left
                  << " / " << t.description << '\n';
    }

    std::cout << "CSV imported id=" << from_csv.id << '\n';
    return 0;
}
