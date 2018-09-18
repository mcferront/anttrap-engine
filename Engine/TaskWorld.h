#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "Threads.h"

class Thread;

typedef void (*PEXECUTE) (const struct TaskExecution &task);

// LOCKS WHEN
// If a task is created and allocs before it is started
//    There is a lock when this TLM is allc'd and freed

// TaskWorldThreads are associated with a HW thread, they run tasks
// TaskGroup handle sync points for task completions
// Tasks can run wide on any thread
   // they have a preference for the parent task thread they were spawned on (if any)
   // If a task thread is blocked waiting
      // it spawns an auxillary worker thread on the same logical processor
      // each time the aux worker finishes a task it checks to see if the parent is still waiting
      // if the parent is ready to go the aux worker ends
      // else the aux worker keeps stealing tasks

// Associate a task with a group
#define TaskCreate(group)\
   Task(group)

#define TaskGroupWait(group)\
   group.Wait();

#define TaskSpawn(task)\
   task.Spawn();

typedef uint64 TaskId;

class TaskGroup;
class TaskWorld;
class TaskWorldThread;

class Task
{
   friend class TaskGroup;
   friend class TaskWorldThread;
   friend class TaskWorld;
   friend struct TaskExecution;

public:
   PEXECUTE Execute;
   void *pData;

   Task( TaskGroup *pGroup );
   Task( void ) {}

   // Task local memory, freed after the task finishes
   void *AllocTLM(
      size_t size
      );

   void Spawn( void );

private:
   TaskGroup *_pGroup;
   TaskId _Id;
   bool _HasTaskWorldMemory;
};

class TaskGroup
{
friend class Task;
friend class TaskWorldThread;

public:
   TaskGroup( void ) { m_TasksRunning = 0; }
   void Wait( void );

   bool NeedsToWait( void ) const { return m_TasksRunning > 0; }

private:
   void *AllocTLM(
      const Task &task,
      size_t size
      );

   void AddTask(
      const Task &task 
      );

   void TaskComplete(
      const Task &task
      );

private:
   uint64 m_TasksRunning;
};

struct TaskExecution
{
   friend class TaskWorldThread;

public:
   // Task local memory, freed after the task finishes
   void *AllocTLM( 
      size_t size
      ) const;
   
   void *pData;

   TaskGroup *Group( void ) const { return _pTask->_pGroup; }

public:
   static TaskExecution Empty;

private:
   Task *_pTask;
   TaskWorldThread *_pThread;
};

class TaskWorld
{
   friend class Task;
   friend class TaskGroup;
   friend class TaskWorldThread;

private:
   static const uint32 MaxTasks = 1024;
   static const uint32 MaxAuxThreads = 32;

public:
   static const uint32 MaxThreads = 32;

public:
   struct Stats
   {
      struct Thread
      {
         uint32 owner;
         uint32 tasks;
         uint32 tlm_total;
         uint32 tlm_used;
      };

      uint32 tlm_used;
      uint32 tlm_free;
      uint32 tlm_blocks;
      uint32 tlm_total;

      uint32 num_threads;

      Thread threads[MaxThreads + MaxAuxThreads];
   };

private:
   struct MemBlock
   {
      size_t size;
      TaskId taskId;
   };

public:
   static TaskWorld &Instance( );

public:
   void Create( 
      uint32 numThreads = 4
      );

   void Destroy( );

   void Run( );

   void QueueTask( 
      const Task &task
      );

   Stats GetStats( void );

   ThreadId GetThreadId( 
       uint32 threadIndex
   );

   const uint32 GetNumThreads( void ) const { return m_NumThreads; }

private:
   Task WaitForTask( 
      bool block = true
      );
      
   // The TaskWorldThread associated with the calling HW thread
   TaskWorldThread *GetTaskWorldThread( );

   void *AllocTLM(
      const Task &task,
      size_t size
      );

   void FreeTLM(
      const Task &task
      );

   TaskWorldThread *AllocAuxThread( void );
   
   void FreeAuxThread(
      TaskWorldThread *pThread
      );

private:
   TaskWorldThread *m_pThreads;
   TaskWorldThread *m_pAuxThreadPool[ MaxAuxThreads ];

   Semaphore m_TaskAddedEvent;
   Semaphore m_AnyTaskEvent;

   Task m_Tasks[ MaxTasks ];

   uint32 m_TaskTail;
   uint32 m_TaskHead;

   Lock m_QueueLock;
   Lock m_AllocLock;

   List<MemBlock*> m_UsedMemBlocks;
   List<MemBlock*> m_FreeMemBlocks;

   uint32 m_NumThreads;

   uint32 m_AuxPoolHead;
   uint32 m_AuxPoolTail;

   bool m_Shutdown;
};


class TaskWorldThread
{
   static const uint32 MemAlignment = 16;
   static const uint32 BlockSize = MemAlignment * 1024;
   static const uint32 MaxTasks = 1024;

   friend class TaskGroup;
   friend class TaskWorld;
   friend struct TaskExecution;

private:
   struct MemHeader
   {
      uint64 taskId;
      uint32 taskWorldThreadId;
      size_t size;
   };

   class HwThread : public Thread
   {
   public:
      TaskWorldThread *m_pInstance;

      virtual void OnThreadRun( void );
   };

private:
   void Create( 
      int taskWorldId
   );

   void CreateAsAuxWorker(
      int logicalProcessor
   );

   void WakeupAuxWorker(
      TaskWorldThread *pOwner,
      const TaskGroup *pGroup
   );

   void Destroy( void );

   // Freed after task finishes
   void *AllocTLM(
      const Task &task,
      size_t size 
   );

   void AddTask( 
      const Task &task
   );

   Task WaitForTask( void );

   bool StealTask(
      Task *pTask
   );

   bool CascadeStealTask(
      Task *pTask
   );

   void SpawnAuxWorker( 
      const TaskGroup *pGroup
   );

   TaskWorldThread *GetActiveThread( void );

   void ThreadUpdate( void );
   void ThreadComplete( void );

   uint32 GetStats( 
      TaskWorld::Stats::Thread *pThreads, 
      uint32 ownerIndex, 
      uint32 currentIndex );


#ifdef ENABLE_TASKWORLD_MEMORY_VALIDATION
   void MarkFreeMemory( size_t startPos, size_t size ) { MarkMemory( startPos, size, 0xf0 ); }
   void MarkAllocatedMemory( size_t startPos, size_t size ) { MarkMemory( startPos, size, 0xa0 ); }

   void MarkMemory( size_t startPos, size_t size, byte b )
   {
      Debug::Assert( Condition(size + startPos <= m_TLMSize), "Invalid Memory Marking" );
      memset( m_pTLM + startPos, b + m_TaskWorldThreadId, size );
   }
#endif //ENABLE_TASKWORLD_MEMORY_VALIDATION

   void Stop( void ) { m_HwThread.Stop( false ); }

   void Run( 
      int logicalProcessor
   )  
   { 
      m_LogicalProcessor = logicalProcessor;
      m_HwThread.m_pInstance = this; 
      m_HwThread.Run( logicalProcessor ); 
   }

   ThreadId GetThreadId( void ) { return m_HwThread.GetThreadId( ); }

private:
   const TaskGroup *m_pGroup;
   TaskWorldThread *m_pAuxWorker;
   TaskWorldThread *m_pOwner;
   ThreadEvent m_AuxWorkerCompleteEvent;
   ThreadEvent m_AuxWorkerReadyEvent;

   Task m_Tasks[ MaxTasks ];
   HwThread m_HwThread;
   uint32 m_TaskWorldThreadId;
   sint32 m_LogicalProcessor;
   TaskId m_ExecutingTaskId;
   Semaphore m_TaskAddedEvent;
   uint32 m_TaskHead;
   uint32 m_TaskTail;
   byte *m_pTLM;
   size_t m_TLMPos;
   size_t m_TLMSize;

   bool m_IsAuxWorker;
};
