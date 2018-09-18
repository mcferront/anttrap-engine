#pragma once

#include "EngineGlobal.h"

class Localization
{
public:
   static Localization &Instance( void );

public:
   Localization( void )
   {}

   void Create ( void );
   void Destroy( void );

   size_t GetString(
      char *pLocalized,
      const char *pKeyword,
      size_t localizedSize
   );

   void SetString( 
      const char *pKeyword, 
      const char *pValue
   );

private:
   size_t InsertRegistryKeys(
      char *pLocalized,
      const char *pString,
      size_t localizedSize,
      size_t currentLength
   );
};
