#pragma once

#include "enkiTS/TaskScheduler.h"

namespace grinliz {

    struct CompletionActionDelete : enki::ICompletable
    {
        enki::Dependency    m_dependency;

        // We override OnDependenciesComplete to provide an 'action' which occurs after
        // the dependency task is complete.
        void OnDependenciesComplete(enki::TaskScheduler* pTaskScheduler_, uint32_t threadNum_)
        {
            // Call base class OnDependenciesComplete BEFORE deleting depedent task or self
            ICompletable::OnDependenciesComplete(pTaskScheduler_, threadNum_);

            //printf("CompletionActionDelete::OnDependenciesComplete called on thread %u\n", threadNum_);

            // In this example we delete the dependency, which is safe to do as the task
            // manager will not dereference it at this point.
            // However the dependency task should have no other dependents,
            // This class can have dependencies.
            delete m_dependency.GetDependencyTask(); // also deletes this as member
        }
    };

    // To use this, simply subclass from SelfDeletingTask, and the rest 
    // is fire and forget. It's very handy.
    struct SelfDeletingTask : enki::ITaskSet
    {
        CompletionActionDelete m_deleter;
        enki::Dependency m_depenency;

        SelfDeletingTask() {
            m_deleter.SetDependency(m_deleter.m_dependency, this);
        }

        virtual ~SelfDeletingTask() {
            //printf("SelfDeletingTask deleted\n");
        }
    };

    class JobHandle {
    public:
        JobHandle() {}
        JobHandle(enki::TaskScheduler* pool, enki::ICompletable* task, bool deleteTask=true) : _deleteTask(deleteTask), _task(task), _pool(pool) {}
        ~JobHandle() {
            Complete();
        }

        void Complete() {
            if (_task) {
                _pool->WaitforTask(_task);
                if (_deleteTask)
                    delete _task;
                _task = 0;
                _pool = 0;
            }
        }
    private:
        bool _deleteTask = true;
        enki::ICompletable* _task = nullptr;
        enki::TaskScheduler* _pool = nullptr;
    };
}