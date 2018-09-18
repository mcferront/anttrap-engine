#include "EnginePch.h"

#include "StringPool.h"

StringPool &StringPool::Instance( void )
{
   static StringPool s_instance;
   return s_instance;
}
