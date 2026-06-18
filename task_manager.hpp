#ifndef TASK_MANAGER_HPP
#define TASK_MANAGER_HPP

struct ManagerTaskConfig_t {
    const char *nombre;
};

void TaskManager(void *pvParameters);

#endif
