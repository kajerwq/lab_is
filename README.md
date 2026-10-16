# TaskFlow IS (вариант Osipov)

Учебный проект по курсу «Программирование информационных систем».

## Отличия варианта

- ЛР1: `description` + `assign()`
- ЛР2: `DueDatePriorityStrategy`, CSV Adapter, stub SQLite, Factory (`mode`)
- ЛР3: расширенные тесты, GitHub Actions + `.gitlab-ci.yml`
- ЛР4: HTTP API (`/health` → `{"ok":true,...}`), Docker, k6

## Сборка

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_HTTP_SERVER=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/taskflow_cli
./build/taskflow_server
```
