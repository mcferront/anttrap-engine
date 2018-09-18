#include "EnginePch.h"

#include "TaskWorld.h"

TaskExecution TaskExecution::Empty;
uint64 UniqueTaskId = 1;

Task::Task( 
   TaskGroup *pGroup 
   )
{
   _Id = (TaskId) AtomicIncrement( &UniqueTaskId );
   _HasTaskWorldMemory = false;
   _pGroup = pGroup;
   Execute = NULL;
   pData = NULL;

   if ( NULL != _pGroup )
      _pGroup->AddTask( *this );
}

// Task local memory, freed after the task finishes
void *Task::AllocTLM( 
   size_t size
   )
{
   _HasTaskWorldMemory = true;
   return TaskWorld::Instance( ).AllocTLM( *this, size );
}

void Task::Spawn( void )
{
   TaskWorld::Instance( ).QueueTask( *this );
}

void TaskGroup::AddTask(
   const Task &task
   )
{
   AtomicIncrement( &m_TasksRunning );
}

void TaskGroup::TaskComplete(
   const Task &task
   )
{
   AtomicDecrement( &m_TasksRunning );
}

// Wait for all spawned tasks
void TaskGroup::Wait( 
   void 
   )
{
   while ( m_TasksRunning > 0 )
   {
      TaskWorldThread *pThread = TaskWorld::Instance( ).GetTaskWorldThread( );
      
      // if we're a task thread and we're supposed to wait
      // then spawn an aux worker thread to complete other tasks
      if ( NULL != pThread )
         pThread->SpawnAuxWorker( this );
      else
         Thread::YieldThread( );
   }
}

void *TaskExecution::AllocTLM( 
   size_t size
   ) const
{
   return _pThread->AllocTLM( *_pTask, size );
}

void TaskWorldThread::Create( 
   int taskWorldId
   )
{
   m_TaskHead = 0;
   m_TaskTail = 0;
   m_ExecutingTaskId = 0;
   m_LogicalProcessor = -1;
   m_TaskWorldThreadId = taskWorldId;

   m_pAuxWorker = NULL;
   m_pOwner = NULL;
   m_pGroup = NULL;
   m_IsAuxWorker = false;

   memset( m_Tasks, 0, sizeof(m_Tasks) );

   m_AuxWorkerCompleteEvent = Thread::CreateEvent( );
   m_TaskAddedEvent = Thread::CreateSemaphore( 0, MaxTasks );

   m_pTLM = (byte *) malloc( BlockSize );
   m_TLMPos = 0;
   m_TLMSize = BlockSize;

#ifdef ENABLE_TASKWORLD_MEMORY_VALIDATION
   MarkFreeMemory( m_TLMPos, m_TLMSize );
#endif
}

void TaskWorldThread::CreateAsAuxWorker(
   int logicalProcessor
   )
{
   Create( -1 );

   m_IsAuxWorker = true;
   m_AuxWorkerReadyEvent = Thread::CreateEvent( );

   Run( logicalProcessor );
}

void TaskWorldThread::WakeupAuxWorker(
   TaskWorldThread *pOwner,
   const TaskGroup *pGroup
)
{
   // we are an aux worker thread
   m_pOwner = pOwner;
   m_pGroup = pGroup;
   m_TaskWorldThreadId = pOwner->m_TaskWorldThreadId << 0xf;

   Thread::Signal( m_AuxWorkerReadyEvent );
}

void TaskWorldThread::Destroy( )
{
   Thread::DeleteSemaphore( m_TaskAddedEvent );
   Thread::DeleteEvent( m_AuxWorkerCompleteEvent );

   if ( true == m_IsAuxWorker )
      Thread::DeleteEvent( m_AuxWorkerReadyEvent );

   // TODO the threads aren't terminating their waits
   // so they won't die
   //m_HwThread.Join( );

   free( m_pTLM );
   m_pTLM = NULL;

   // TODO: We shouldn't call Terminate here
   // we should call Join, but the waiting threads
   // are blocked until their calling thread is signaled or destroyed
   // and I don't have a good solution for rearchitecting that here
   m_HwThread.Terminate( );
}

// Task local memory Freed after task finishes
void *TaskWorldThread::AllocTLM( 
   const Task &task,
   size_t size 
   )
{
   Debug::Assert( Condition(Thread::GetCurrentThreadId() == GetThreadId()), "Incorrect thread for TLM" );
   Debug::Assert( Condition(m_ExecutingTaskId == task._Id), "Incorrect task alloc'ing" );

#ifdef ENABLE_TASKWORLD_MEMORY_VALIDATION
   size += sizeof(MemHeader);
#endif

   size = Align(size, MemAlignment);

   if ( m_TLMSize - m_TLMPos < size )
   {
      size_t required = size - (m_TLMSize - m_TLMPos);

      m_TLMSize += Align(required, BlockSize);
      m_TLMSize += Align(m_TLMSize, MemAlignment);
      
      m_pTLM = (byte *) realloc( m_pTLM, m_TLMSize );
   #ifdef ENABLE_TASKWORLD_MEMORY_VALIDATION
      MarkFreeMemory( m_TLMPos, m_TLMSize - m_TLMPos );
   #endif
   }

   size_t pos = m_TLMPos;

#ifdef ENABLE_TASKWORLD_MEMORY_VALIDATION
   MarkAllocatedMemory( m_TLMPos, size );
#endif

   m_TLMPos += size;

#ifdef ENABLE_TASKWORLD_MEMORY_VALIDATION
   MemHeader *pHeader = (MemHeader *)(m_pTLM + pos);
   pHeader->taskId = task._Id;
   pHeader->taskWorldThreadId = m_TaskWorldThreadId;
   pHeader->size = size - sizeof(MemHeader);

   pos += sizeof(MemHeader);
#endif

   return m_pTLM + pos;
}

void TaskWorldThread::AddTask( 
   const Task &task
   )
{
   uint32 index = (AtomicIncrement(&m_TaskTail) - 1) % MaxTasks;
   Debug::Assert( Condition(m_Tasks[ index ]._Id == 0), "Task queue wrapped" ); 

   m_Tasks[ index ] = task;

   // signal that a task has been added
   Thread::Signal( m_TaskAddedEvent );
}

Task TaskWorldThread::WaitForTask( void )
{
   Task task;

   if ( true == StealTask( &task ) )
      return task;

   // if we have no owner and we've gotten here it means
   // we have no aux tasks, so we can block until a task is queued
   
   // if we have an owner and we are here it means we're an aux thread
   // and the owner is blocked waiting on some tasks to complete
   // so we don't block here, if no task is queued
   // then we'll continue out and exit after the thread loop
   bool block = m_pOwner == NULL;

   return TaskWorld::Instance( ).WaitForTask( block );
}

bool TaskWorldThread::StealTask(
   Task *pTask
   )
{
   bool success = Thread::Wait( m_TaskAddedEvent, 0 );

   // if success there is a task for us (there couple be multiple threads here with multiple tasks
   // but we know there is at least one for us, so we can proceed
   if ( true == success )
   {
      uint32 index = (AtomicIncrement(&m_TaskHead) - 1) % MaxTasks;
      Debug::Assert( Condition(m_Tasks[ index ]._Id != 0), "Task not queued" ); 

      *pTask = m_Tasks[ index ];
      m_Tasks[ index ]._Id = 0;
   }

   return success;
}

bool TaskWorldThread::CascadeStealTask(
   Task *pTask
)
{
   // cascade up to the owner and steal the first queued task we can find
   TaskWorldThread *pActive = GetActiveThread( );

   bool success;

   do
   {
      success = pActive->StealTask( pTask );

      if ( true == success )
         break;

      pActive = pActive->m_pOwner;
   }
   while ( NULL != pActive );

   return success;
}

void TaskWorldThread::SpawnAuxWorker( 
   const TaskGroup *pGroup
)
{
   Debug::Assert( Condition(NULL == m_pAuxWorker), "Spawn AuxWorker already had an AuxWorker" );

   m_pAuxWorker = TaskWorld::Instance( ).AllocAuxThread( );   
   m_pAuxWorker->WakeupAuxWorker( this, pGroup );

   Thread::Wait( m_AuxWorkerCompleteEvent );
}

TaskWorldThread *TaskWorldThread::GetActiveThread( void )
{
   // If we have an aux worker then we are blocked and
   // we've spawned it to help with tasks, 
   // thus it is the active thread in our chain, not us
   if ( NULL != m_pAuxWorker )
      return m_pAuxWorker->GetActiveThread( );

   return this;
}

void TaskWorldThread::ThreadUpdate( void )
{
   // Wait for aux worker to be called
   if ( true == m_IsAuxWorker )
   {
      if ( false == Thread::Wait(m_AuxWorkerReadyEvent) )
         return;
   }

   Task task = WaitForTask( );

   m_ExecutingTaskId = task._Id;

   TaskExecution te;
   te.pData = task.pData;
   te._pTask = &task;
   te._pThread = this;

   if ( NULL != task.Execute )
      task.Execute( te );

#ifdef ENABLE_TASKWORLD_MEMORY_VALIDATION
   if ( m_TLMPos > 0 )
   {
      size_t pos = 0;

      while (pos < m_TLMPos)
      {
         MemHeader *pHeader = (MemHeader *)(m_pTLM + pos);
         Debug::Assert( Condition(pHeader->taskId == m_ExecutingTaskId), "TLM Memory Corruption" );
         Debug::Assert( Condition(pHeader->taskWorldThreadId == m_TaskWorldThreadId), "TLM Memory Corruption" );

         pos += pHeader->size + sizeof(MemHeader);
      }

      Debug::Assert( Condition(pos == m_TLMPos), "TLM Memory Corruption" );
   }
   MarkFreeMemory( 0, m_TLMSize );
#endif //ENABLE_TASKWORLD_MEMORY_VALIDATION

   m_TLMPos = 0;
   m_ExecutingTaskId = 0;

   if ( task._HasTaskWorldMemory )
      TaskWorld::Instance( ).FreeTLM( task );

   if ( NULL != task._pGroup )
      task._pGroup->TaskComplete( task );

   // if we're an aux worker...
   if ( true == m_IsAuxWorker )
   {
      // owner's group no longer needs to wait?
      if ( false == m_pGroup->NeedsToWait( ) )
      {
         Debug::Assert( Condition(m_pOwner->m_pAuxWorker == this), "AuxWorker is not 'this'" );

         // we are no longer an aux worker of our owner
         m_pOwner->m_pAuxWorker = NULL;

         // tell the owner to continue
         Thread::Signal( m_pOwner->m_AuxWorkerCompleteEvent );
      }
      else
         // tell us to continue
         Thread::Signal( m_AuxWorkerReadyEvent );
   }
}

void TaskWorldThread::ThreadComplete( void )
{
   // if we're an aux worker, go back into the pool
   if ( NULL != m_pOwner )
      TaskWorld::Instance( ).FreeAuxThread( this );
}

uint32 TaskWorldThread::GetStats( 
   TaskWorld::Stats::Thread *pThreads, 
   uint32 ownerIndex, 
   uint32 currentIndex )
{
   pThreads[ currentIndex ].owner = ownerIndex;
   pThreads[ currentIndex ].tasks = m_TaskTail - m_TaskHead;
   pThreads[ currentIndex ].tlm_total = m_TLMSize;
   pThreads[ currentIndex ].tlm_used = m_TLMPos;

   int numAuxThreads = 0;

   if ( m_pAuxWorker )
      numAuxThreads = m_pAuxWorker->GetStats( pThreads, currentIndex, currentIndex + 1 );

   return numAuxThreads + 1;
}

void TaskWorldThread::HwThread::OnThreadRun( void )
{
   while ( ShouldRun( ) )
      m_pInstance->ThreadUpdate( );

   m_pInstance->ThreadComplete( );
}

TaskWorld &TaskWorld::Instance( )
{
   static TaskWorld s_instance;
   return s_instance;
}

void TaskWorld::Create( 
   uint32 numThreads// = 4
   )
{
   Debug::Assert( Condition(m_NumThreads <= MaxThreads), "Too many threads for TaskWorld" );
   Debug::Assert( Condition(sizeof(TaskWorldThread::MemHeader) % TaskWorldThread::MemAlignment == 0), "Incorrect MemHeader alignment" );

   m_TaskTail = 0;
   m_TaskHead = 0;

   m_NumThreads = numThreads;
   m_pThreads = new TaskWorldThread[ m_NumThreads ];

   m_UsedMemBlocks.Create( );
   m_FreeMemBlocks.Create( );

   for ( uint32 i = 0; i < m_NumThreads; i++ )
      m_pThreads[ i ].Create( (i + 1) );

   m_TaskAddedEvent = Thread::CreateSemaphore( 0, MaxTasks );
   m_AnyTaskEvent   = Thread::CreateSemaphore( 0, MaxTasks );

   for ( uint32 i = 0; i < MaxAuxThreads; i++ )
   {
      m_pAuxThreadPool[ i ] = new TaskWorldThread;
      m_pAuxThreadPool[ i ]->CreateAsAuxWorker( (i + 1) % m_NumThreads );
   }

   m_AuxPoolHead = 0;
   m_AuxPoolTail = 0;

   memset( m_Tasks, 0, sizeof(m_Tasks) );
}

void TaskWorld::Destroy( )
{
   m_Shutdown = true;

   Thread::DeleteSemaphore( m_TaskAddedEvent );
   Thread::DeleteSemaphore( m_AnyTaskEvent );

   for ( uint32 i = 0; i < MaxAuxThreads; i++ )
      m_pAuxThreadPool[ i ]->Stop( );

   for ( uint32 i = 0; i < m_NumThreads; i++ )
      m_pThreads[ i ].Stop( );

   for ( uint32 i = 0; i < MaxAuxThreads; i++ )
   {
      m_pAuxThreadPool[ i ]->Destroy( );
      delete m_pAuxThreadPool[ i ];
   }

   for ( uint32 i = 0; i < m_NumThreads; i++ )
      m_pThreads[ i ].Destroy( );

   delete [] m_pThreads;
   
   m_pThreads = NULL;
   m_NumThreads = 0;

   m_UsedMemBlocks.Destroy( );
   m_FreeMemBlocks.Destroy( );
}

void TaskWorld::Run( )
{
   // TODO: be smarter about which processor to run on
   for ( uint32 i = 0; i < m_NumThreads; i++ )
      m_pThreads[ i ].Run( i + 1 );
}

void TaskWorld::QueueTask(
   const Task &task
   )
{
   TaskWorldThread *pThread = GetTaskWorldThread( );
   
   if ( NULL != pThread )
      pThread->AddTask( task );
   else
   {
      uint32 index = (AtomicIncrement(&m_TaskTail) - 1) % MaxTasks;
      Debug::Assert( Condition(m_Tasks[ index ]._Id == 0), "Task queue wrapped" ); 

      m_Tasks[ index ] = task;

      Thread::Signal( m_TaskAddedEvent );
   }

   Thread::Signal( m_AnyTaskEvent );
}

TaskWorld::Stats TaskWorld::GetStats( void )
{
   Stats stats = { 0 };

   {
      ScopeLock lock( m_AllocLock );

      for ( uint32 i = 0; i < m_UsedMemBlocks.GetSize(); i++ )
         stats.tlm_used += m_UsedMemBlocks.GetAt(i)->size + sizeof(MemBlock);

      for ( uint32 i = 0; i < m_FreeMemBlocks.GetSize(); i++ )
         stats.tlm_free += m_FreeMemBlocks.GetAt(i)->size + sizeof(MemBlock);

      stats.tlm_blocks = m_UsedMemBlocks.GetSize() + m_FreeMemBlocks.GetSize();
      stats.tlm_total = stats.tlm_used + stats.tlm_free;
   }

   for ( uint32 i = 0; i < m_NumThreads; i++ )
      stats.num_threads += m_pThreads[ i ].GetStats( stats.threads, -1, stats.num_threads ); 

   return stats;
}

ThreadId TaskWorld::GetThreadId( uint32 threadIndex )
{
    Debug::Assert( Condition(threadIndex < m_NumThreads), "ThreadIndex is invalid" );
    return m_pThreads[ threadIndex ].GetThreadId( );
}

Task TaskWorld::WaitForTask( 
   bool block // = true
   )
{
   //todo future - tbb style
   //tbb adds tasks in this order:
   //priority 1 - a task started from an executing task (on the thread's queue)
   //priority 2 - the next task queued by the previous task's parent (on the thread's queue)
   //priority 3 - the next task on the thread's queue
   //priority 4 - a task in the shared queue with affinity for the thread
   //priority 5 - the next task in the shared queue 
   //priority 6 - the first task from another's thread queue

   //how it works now
   //priority 1 - the last task queued from the same dedicated task thread
   //priority 2 - the first task queued from another dedicated task thread's queue
   //priority 3 - the last task queued from a non dedicated task thread

   Task task( NULL );

   while ( true )
   {
      // try and steal a task queued with the active thread in each thread chain
      for ( uint32 i = 0; i < m_NumThreads; i++ )
      {
         if ( m_pThreads[ i ].GetActiveThread( )->StealTask( &task ) )
            return task;
      }

      // if no queued task with the active thread in each thread chain then
      // try to steal from any of the threads
      for ( uint32 i = 0; i < m_NumThreads; i++ )
      {
         if ( m_pThreads[ i ].CascadeStealTask( &task ) )
            return task;
      }

      {
         bool success = Thread::Wait( m_TaskAddedEvent, 0 );

         if ( true == success )
         {
            uint32 index = (AtomicIncrement(&m_TaskHead) - 1) % MaxTasks;
            Debug::Assert( Condition(m_Tasks[ index ]._Id != 0), "Task not queued" ); 

            task = m_Tasks[ index ];
            m_Tasks[ index ]._Id = 0;

            return task;
         }
      }

      if ( false == block )
         break;

      // When this fires there is no guarantee this waiting thread will get it,
      // it could be picked up by anyone when this event fires
      Thread::Wait( m_AnyTaskEvent, 1000 );

      if ( m_Shutdown )
         break;
   }

   return Task(NULL);
}

TaskWorldThread *TaskWorld::GetTaskWorldThread( )
{
   ThreadId current = Thread::GetCurrentThreadId( );

   for ( uint32 i = 0; i < m_NumThreads; i++ )
   {
      TaskWorldThread *pThread = m_pThreads[ i ].GetActiveThread( );

      if ( current == pThread->GetThreadId( ) )
         return m_pThreads[ i ].GetActiveThread( );
   }

   return NULL;
}

void *TaskWorld::AllocTLM(
   const Task &task,
   uint32 size 
   )
{
   ScopeLock lock( m_AllocLock );

      // if debug memory always alloc/free through the memory manager
      // this way we use its full memory validation
   #ifdef ENABLE_DEBUG_TASKWORLD_MEMORY
      MemBlock *pBlock = (MemBlock *) malloc( sizeof(MemBlock) + size );
      pBlock->size = size;
      pBlock->taskId = task._Id;
   #else
      // if not debug memory then track it ourselves
      MemBlock *pBlock;

      uint32 i, n;

      for ( i = 0, n = m_FreeMemBlocks.GetSize( ); i < n; i++ )
      {
         pBlock = m_FreeMemBlocks.GetAt( i );

         if ( pBlock->size >= size )
            break;
      }

      if ( i < m_FreeMemBlocks.GetSize( ) )
         m_FreeMemBlocks.RemoveAt( i );
      else
      {
         pBlock = (MemBlock *) malloc( sizeof(MemBlock) + size );
         pBlock->size = size;
      }

      pBlock->taskId = task._Id;
   #endif

   m_UsedMemBlocks.Add( pBlock );
   return (byte *) pBlock + sizeof(MemBlock);
}

void TaskWorld::FreeTLM(
   const Task &task
   )
{
   ScopeLock lock( m_AllocLock );

   for ( uint32 i = 0; i < m_UsedMemBlocks.GetSize(); i++ )
   {
      if ( m_UsedMemBlocks.GetAt(i)->taskId == task._Id )
      {
         #ifdef ENABLE_DEBUG_TASKWORLD_MEMORY
            free( m_UsedMemBlocks.GetAt( i ) );
         #else
            m_FreeMemBlocks.Add( m_UsedMemBlocks.GetAt(i) );
         #endif
         
         m_UsedMemBlocks.RemoveAt( i );

         --i;
      }
   }
}

TaskWorldThread *TaskWorld::AllocAuxThread( void )
{
   uint32 index = (AtomicIncrement(&m_AuxPoolTail) - 1) % MaxAuxThreads;
   
   TaskWorldThread *pThread = m_pAuxThreadPool[ index ];
   Debug::Assert( Condition(NULL != pThread), "Aux Thread Slot is full" );

   m_pAuxThreadPool[ index ] = NULL;

   return pThread;
}

void TaskWorld::FreeAuxThread( 
   TaskWorldThread *pThread
   )
{
   uint32 index = (AtomicIncrement(&m_AuxPoolHead) - 1) % MaxAuxThreads;
   Debug::Assert( Condition(NULL == m_pAuxThreadPool[index]), "Aux Thread Slot should be empty" );

   m_pAuxThreadPool[ index ] = pThread;
}
