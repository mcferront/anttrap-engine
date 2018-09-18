#include "EnginePch.h"

#include "SystemId.h"

HashTable<const char *, Id::Registrar::IdDesc> *Id::Registrar::m_pIdHash;
Lock Id::Registrar::m_Lock;