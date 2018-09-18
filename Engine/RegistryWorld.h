#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "StringPool.h"

class Registry;
typedef void( *P_REGISTERY_VALUE_CALLBACK ) ( const char *pKey );

//very important!
//do not add any member variables to the derived classes
//or they can not be added by value to the hash table
//right now the derived classes simply act as wrapper functions
//for the base class
class RegistryValue
{
   friend class RegistrySerializer;
   friend class RegistryInt;
   friend class RegistryBool;
   friend class RegistryString;
   friend class RegistryFloat;
   friend class RegistryMethod;

public:
   RegistryValue( void )
   {
      m_Type = Invalid;
      m_Callback = NULL;
   }

   enum Type
   {
      Invalid = -1,
      Int = 1,
      Float = 2,
      String = 3,
      Bool = 4,
      Method = 5,
   };

private:
   typedef void( *PMETHOD ) ( const char *pArgs[ ], int numArgs );
   union
   {
      int m_Int;
      bool m_Bool;
      float m_Float;
      const char *m_pString;
      PMETHOD m_Method;
   };

   union
   {
      int m_dMin;
      float m_fMin;
   };

   union
   {
      int m_dMax;
      float m_fMax;
   };

   Type m_Type;
   const char *m_pPath;
   P_REGISTERY_VALUE_CALLBACK m_Callback;

private:
   RegistryValue(
      int value,
      const char *pPath
   )
   {
      m_Int = value;
      m_Type = Int;
      m_pPath = StringRef(pPath);
      m_Callback = NULL;
   }

   RegistryValue(
      float value,
      const char *pPath
   )
   {
      m_Float = value;
      m_Type = Float;
      m_pPath = StringRef(pPath);
      m_Callback = NULL;
   }

   RegistryValue(
      bool value,
      const char *pPath
   )
   {
      m_Bool = value;
      m_Type = Bool;
      m_pPath = StringRef(pPath);
      m_Callback = NULL;
   }

   RegistryValue(
      PMETHOD value,
      const char *pPath
   )
   {
      m_Method = value;
      m_Type = Method;
      m_pPath = StringRef(pPath);
      m_Callback = NULL;
   }

   RegistryValue(
      const char *pValue,
      const char *pPath
   );

public:
   Type GetType( void ) const { return m_Type; }
};

class RegistryWorld
{
   friend class RegistryInt;
   friend class RegistryBool;
   friend class RegistryString;
   friend class RegistryFloat;
   friend class RegistryMethod;

public:
   static RegistryWorld &Instance( void );

private:
   HashTable<const char *, RegistryValue *> m_Hashtable;

public:
   void Create( void )
   {
      m_Hashtable.Create( );
   }

   void Destroy( void );

   void ImportRegistry(
      Registry *pRegistry
   );

   void ExportRegistry(
      const char *pRootPath,
      Registry *pRegistry
   );

   void RemoveEntries(
      const char *pRootPath
   );

   Enumerator<const char *, RegistryValue *> Enumerate( void )
   {
      return m_Hashtable.GetEnumerator( );
   }

   RegistryValue *GetValue(
      const char *pPath,
      bool create
   );
};

class RegistryInt
{

private:
   RegistryValue * m_pValue;

public:
   RegistryInt(
      const char *pPath,
      int value = 0,
      int min = -INT_MAX,
      int max = INT_MAX,
      P_REGISTERY_VALUE_CALLBACK valueChanged = NULL
   )
   {
      m_pValue = RegistryWorld::Instance( ).GetValue( pPath, true );

      if ( RegistryValue::Invalid == m_pValue->m_Type )
      {
         m_pValue->m_Type = RegistryValue::Int;
         m_pValue->m_dMin = min;
         m_pValue->m_dMax = max;
         m_pValue->m_pPath = pPath;
         SetValue( value );

         m_pValue->m_Callback = valueChanged;
      }

      Debug::Assert( Condition( RegistryValue::Int == m_pValue->m_Type ), "RegistryInt was told to get registry type: %d", m_pValue->m_Type );
   }

   RegistryInt(
      RegistryValue *pValue
   )
   {
      m_pValue = pValue;
      Debug::Assert( Condition( RegistryValue::Int == m_pValue->m_Type ), "RegistryInt was told to get registry type: %d", m_pValue->m_Type );
   }

   void SetValue( int value )
   {
      if ( m_pValue->m_Int == value )
         return;

      m_pValue->m_Int = Math::Clamp( value, m_pValue->m_dMin, m_pValue->m_dMax );
      if ( m_pValue->m_Callback ) m_pValue->m_Callback( m_pValue->m_pPath );
   }

   int  GetValue( void ) const { return m_pValue->m_Int; }
};

class RegistryBool
{
private:
   RegistryValue * m_pValue;

public:
   RegistryBool(
      const char *pPath,
      bool value = false,
      P_REGISTERY_VALUE_CALLBACK valueChanged = NULL
   )
   {
      m_pValue = RegistryWorld::Instance( ).GetValue( pPath, true );

      if ( RegistryValue::Invalid == m_pValue->m_Type )
      {
         m_pValue->m_Type = RegistryValue::Bool;
         m_pValue->m_pPath = pPath;
         m_pValue->m_Bool = value;
         m_pValue->m_Callback = valueChanged;
      }

      Debug::Assert( Condition( RegistryValue::Bool == m_pValue->m_Type ), "RegistryBool was told to get registry type: %d", m_pValue->m_Type );
   }

   RegistryBool(
      RegistryValue *pValue
   )
   {
      m_pValue = pValue;
      Debug::Assert( Condition( RegistryValue::Bool == m_pValue->m_Type ), "RegistryBool was told to get registry type: %d", m_pValue->m_Type );
   }

   void SetValue( bool value ) 
   { 
      if ( m_pValue->m_Bool == value )
         return;
      
      m_pValue->m_Bool = value; 
      if ( m_pValue->m_Callback ) m_pValue->m_Callback( m_pValue->m_pPath );
   }

   bool GetValue( void ) const { return m_pValue->m_Bool; }
};

class RegistryFloat
{
private:
   RegistryValue * m_pValue;

public:
   RegistryFloat(
      const char *pPath,
      float value = 0.0f,
      float min = -FLT_MAX,
      float max = FLT_MAX,
      P_REGISTERY_VALUE_CALLBACK valueChanged = NULL
   )
   {
      m_pValue = RegistryWorld::Instance( ).GetValue( pPath, true );

      if ( RegistryValue::Invalid == m_pValue->m_Type )
      {
         m_pValue->m_Type = RegistryValue::Float;
         m_pValue->m_fMin = min;
         m_pValue->m_fMax = max;
         m_pValue->m_pPath = pPath;
         SetValue( value );
         m_pValue->m_Callback = valueChanged;
      }

      Debug::Assert( Condition( RegistryValue::Float == m_pValue->m_Type ), "RegistryFloat was told to get registry type: %d", m_pValue->m_Type );
   }

   RegistryFloat(
      RegistryValue *pValue
   )
   {
      m_pValue = pValue;
      Debug::Assert( Condition( RegistryValue::Float == m_pValue->m_Type ), "RegistryFloat was told to get registry type: %d", m_pValue->m_Type );
   }

   void SetValue( float value )
   {
      if ( m_pValue->m_Float == value )
         return;

      m_pValue->m_Float = Math::Clamp( value, m_pValue->m_fMin, m_pValue->m_fMax );
      if ( m_pValue->m_Callback ) m_pValue->m_Callback( m_pValue->m_pPath );
   }

   float GetValue( void ) const { return m_pValue->m_Float; }
};

class RegistryString
{
private:
   RegistryValue * m_pValue;

public:
   RegistryString(
      const char *pPath,
      const char *pString = "",
      P_REGISTERY_VALUE_CALLBACK valueChanged = NULL
   )
   {
      m_pValue = RegistryWorld::Instance( ).GetValue( pPath, true );

      if ( RegistryValue::Invalid == m_pValue->m_Type )
      {
         m_pValue->m_Type = RegistryValue::String;
         m_pValue->m_pPath = pPath;
         m_pValue->m_pString = StringRef( pString );
         m_pValue->m_Callback = valueChanged;
      }

      Debug::Assert( Condition( RegistryValue::String == m_pValue->m_Type ), "RegistryString was told to get registry type: %d", m_pValue->m_Type );
   }

   RegistryString(
      RegistryValue *pValue
   )
   {
      m_pValue = pValue;
      Debug::Assert( Condition( RegistryValue::String == m_pValue->m_Type ), "RegistryString was told to get registry type: %d", m_pValue->m_Type );
   }

   void SetValue( const char *pValue ) 
   { 
      StringRefValidate( pValue ); 
      
      pValue = StringRef( pValue );
      
      if ( m_pValue->m_pString == pValue )
         return;

      m_pValue->m_pString = pValue;  
      if ( m_pValue->m_Callback ) m_pValue->m_Callback( m_pValue->m_pPath );
   }
   
   const char *GetValue( void ) const { return m_pValue->m_pString; }
};

class RegistryMethod
{
private:
   RegistryValue * m_pValue;

public:
   RegistryMethod(
      const char *pPath,
      RegistryValue::PMETHOD method = NULL,
      P_REGISTERY_VALUE_CALLBACK valueChanged = NULL
   )
   {
      m_pValue = RegistryWorld::Instance( ).GetValue( pPath, true );

      if ( RegistryValue::Invalid == m_pValue->m_Type )
      {
         m_pValue->m_Type = RegistryValue::Method;
         m_pValue->m_pPath = pPath;
         m_pValue->m_Method = method;
         m_pValue->m_Callback = valueChanged;
      }

      Debug::Assert( Condition( RegistryValue::Method == m_pValue->m_Type ), "RegistryMethod was told to get registry type: %d", m_pValue->m_Type );
   }

   RegistryMethod(
      RegistryValue *pValue
   )
   {
      m_pValue = pValue;
      Debug::Assert( Condition( RegistryValue::Method == m_pValue->m_Type ), "RegistryMethod was told to get registry type: %d", m_pValue->m_Type );
   }

   void SetValue( RegistryValue::PMETHOD value ) 
   { 
      if ( m_pValue->m_Method == value )
         return;

      m_pValue->m_Method = value;  
   
      if ( m_pValue->m_Callback ) m_pValue->m_Callback( m_pValue->m_pPath );
   }

   RegistryValue::PMETHOD GetValue( void ) const { return m_pValue->m_Method; }
};