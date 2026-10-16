#include "simple_test.hpp"
#include "taskflow/csv_task_adapter.hpp"
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/repository_factory.hpp"
#include "taskflow/sqlite_task_repository.hpp"
#include "taskflow/task_service.hpp"
#include <memory>
#include <string>

using namespace taskflow;

TEST_CASE("factory memory ok") {
    Config cfg;
    cfg.mode = "memory";
    auto repo = make_repository(cfg);
    Task draft;
    draft.title = "f";
    draft.assignee = "u";
    draft.priority = 3;
    auto created = repo->create(draft);
    CHECK_TRUE(repo->find_by_id(created.id).has_value());
}

TEST_CASE("closed tasks go last") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    TaskService svc(repo, std::make_shared<ClosedLastPriorityStrategy>());
    auto a = svc.create_task("A", "u", 1, "", 3);
    auto b = svc.create_task("B", "u", 5, "", 3);
    svc.transition(a.id, TaskStatus::InProgress);
    svc.transition(a.id, TaskStatus::Done);
    auto ordered = svc.list_by_priority();
    CHECK_EQ(ordered.front().id, b.id);
}

TEST_CASE("factory sqlite stub") {
    Config cfg;
    cfg.mode = "sqlite";
    cfg.sqlite_file = "tmp_osipov.db";
    auto repo = make_repository(cfg);
    auto* s = dynamic_cast<SQLiteTaskRepository*>(repo.get());
    CHECK_TRUE(s != nullptr);
    CHECK_EQ(s->path(), std::string("tmp_osipov.db"));
}

TEST_CASE("factory unknown mode throws") {
    Config cfg;
    cfg.mode = "redis";
    CHECK_THROWS(make_repository(cfg));
}

TEST_CASE("due date strategy prefers closer deadline") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    TaskService svc(repo, std::make_shared<DueDatePriorityStrategy>());
    auto far = svc.create_task("far", "u", 3, "", 20);
    auto near = svc.create_task("near", "u", 3, "", 1);
    auto ordered = svc.list_by_priority();
    CHECK_EQ(ordered.front().id, near.id);
    CHECK_EQ(ordered.back().id, far.id);
}

TEST_CASE("csv adapter parses line") {
    auto t = CsvTaskAdapter::from_csv_line("Import;Osipov;2;from csv");
    CHECK_EQ(t.title, std::string("Import"));
    CHECK_EQ(t.assignee, std::string("Osipov"));
    CHECK_EQ(t.priority, 2);
    CHECK_EQ(t.description, std::string("from csv"));
}

TEST_CASE("csv adapter bad line throws") {
    CHECK_THROWS(CsvTaskAdapter::from_csv_line("only-one-field"));
}

TEST_CASE("sqlite stub create throws") {
    SQLiteTaskRepository repo("x.db");
    CHECK_THROWS(repo.create(Task{}));
}
