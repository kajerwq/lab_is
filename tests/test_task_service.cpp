#include "simple_test.hpp"
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"
#include <memory>
#include <string>

using namespace taskflow;

static TaskService make_svc() {
    return TaskService(std::make_shared<InMemoryTaskRepository>(),
                       std::make_shared<DefaultPriorityStrategy>());
}

TEST_CASE("create stores task") {
    auto svc = make_svc();
    auto t = svc.create_task("Report", "Osipov", 2);
    CHECK_TRUE(t.id > 0);
    CHECK_EQ(t.status, TaskStatus::New);
}

TEST_CASE("bad priority throws") {
    auto svc = make_svc();
    CHECK_THROWS(svc.create_task("x", "y", 9));
}

TEST_CASE("workflow New-InProgress-Done") {
    auto svc = make_svc();
    auto t = svc.create_task("Work", "Osipov", 1);
    CHECK_TRUE(svc.transition(t.id, TaskStatus::InProgress));
    CHECK_TRUE(svc.transition(t.id, TaskStatus::Done));
    CHECK_TRUE(!svc.transition(t.id, TaskStatus::InProgress));
}

// ЛР1
TEST_CASE("assign works for open task") {
    auto svc = make_svc();
    auto t = svc.create_task("Bug", "A", 2);
    CHECK_TRUE(svc.assign(t.id, "B"));
    CHECK_EQ(svc.find(t.id)->assignee, std::string("B"));
}

TEST_CASE("assign fails when Done") {
    auto svc = make_svc();
    auto t = svc.create_task("Bug", "A", 2);
    svc.transition(t.id, TaskStatus::InProgress);
    svc.transition(t.id, TaskStatus::Done);
    CHECK_TRUE(!svc.assign(t.id, "B"));
}

TEST_CASE("description > 1000 rejected") {
    auto svc = make_svc();
    CHECK_THROWS(svc.create_task("t", "a", 2, std::string(1001, 'z')));
}

TEST_CASE("description = 1000 ok") {
    auto svc = make_svc();
    auto t = svc.create_task("t", "a", 2, std::string(1000, 'z'));
    CHECK_EQ(t.description.size(), static_cast<std::size_t>(1000));
}

// ЛР3
TEST_CASE("empty title rejected") {
    auto svc = make_svc();
    CHECK_THROWS(svc.create_task("", "a", 2));
}

TEST_CASE("empty assignee rejected") {
    auto svc = make_svc();
    CHECK_THROWS(svc.create_task("t", "", 2));
}

TEST_CASE("priority 0 rejected") {
    auto svc = make_svc();
    CHECK_THROWS(svc.create_task("t", "a", 0));
}

TEST_CASE("priority 6 rejected") {
    auto svc = make_svc();
    CHECK_THROWS(svc.create_task("t", "a", 6));
}

TEST_CASE("cancel from New allowed") {
    auto svc = make_svc();
    auto t = svc.create_task("t", "a", 3);
    CHECK_TRUE(svc.transition(t.id, TaskStatus::Cancelled));
}

TEST_CASE("assign empty throws") {
    auto svc = make_svc();
    auto t = svc.create_task("t", "a", 3);
    CHECK_THROWS(svc.assign(t.id, ""));
}

TEST_CASE("unknown id not found") {
    auto svc = make_svc();
    CHECK_TRUE(!svc.find(12345).has_value());
}

TEST_CASE("repo integration create-find-update-list") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    Task d;
    d.title = "R";
    d.assignee = "O";
    d.priority = 2;
    auto created = repo->create(d);
    CHECK_TRUE(repo->find_by_id(created.id).has_value());
    created.status = TaskStatus::InProgress;
    CHECK_TRUE(repo->update(created));
    CHECK_TRUE(repo->list().size() == 1);
}

TEST_CASE("list order by default score") {
    auto svc = make_svc();
    auto low = svc.create_task("low", "a", 5);
    auto high = svc.create_task("high", "b", 1);
    auto ordered = svc.list_by_priority();
    // score = 10 - priority, меньший score первым
    CHECK_EQ(ordered.front().id, low.id);
    CHECK_EQ(ordered.back().id, high.id);
}
