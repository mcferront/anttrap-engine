#pragma once

#include "EngineGlobal.h"
#include "ResourceWorld.h"
#include "Component.h"
#include "ScriptInstance.h"

class ScriptComponent : public Component
{
    friend class ScriptComponentSerializer;

private:
    struct EventBinding
    {
        Id sender;
        const char *pEvent;
    };

    struct EventDesc
    {
        Id sender;
        Id method;
    };


    static inline uint32 EventBindingHash( 
        EventBinding eb
        )
    {
        return HashFunctions::NUIntHash( (nuint) eb.sender.ToString() );
    }

    static inline bool EventBindingCompare(
        EventBinding eb1,
        EventBinding eb2
        )
    {
        return eb1.sender == eb2.sender && StringRefEqual(eb1.pEvent, eb2.pEvent);
    }

private:
    ScriptInstance *m_pScript;
    ResourceHandle  m_ScriptHandle;

    HashTable<EventBinding, EventDesc> m_Events;
    MemoryStream m_Stream;

public:
    DeclareComponentType(ScriptComponent);

    void Create(
        ResourceHandle script
        );

    virtual void Bind( void );
    
    virtual void Tick(
        float deltaSeconds
        );

    virtual void PostTick( void );

    virtual void Final( void );

    virtual void EditorRender( void );

    void Destroy( void );

    void AddEventListener(
        Id id,
        const char *pEventName,
        const char *pMethod
        );

    void RemoveEventListener(
        Id id,
        const char *pEventName,
        const char *pMethod
        );

    bool SetFloat(
        float value, 
        const char *pFieldName
        ) { return false; }

    bool SetInt(
        int value, 
        const char *pFieldName
        ) { return false; }

    bool SetBool(
        bool value, 
        const char *pFieldName
        ) { return false; }

    bool SetString(
        const char *value, 
        const char *pFieldName
        ) { return false; }

    bool SetTransform(
        const Transform &value, 
        const char *pFieldName
        ) { return false; }

    bool SetMatrix(
        const Matrix &value, 
        const char *pFieldName
        ) { return false; }

    bool SetVector(
        const Vector &value, 
        const char *pFieldName
        ) { return false; }

    bool SetVector2(
        const Vector2 &value, 
        const char *pFieldName
        ) { return false; }

    bool SetColor(
        const Color &value,
        const char *pFieldName
        ) { return false; }

    bool SetId(
        Id value, 
        const char *pFieldName
        ) { return false; }

    const char *GetObjectName( void ) const { return m_pScript->GetObjectName(); }

    ResourceHandle GetScript( void ) const { return m_ScriptHandle; }

private:
    void LoadScript( void );

    void OnEvent(
        const Channel *pSender,
        const char *pEvent,
        const ArgList &list
        );

    void OnLuaScriptReloaded(
        const Channel *pSender,
        const char *pEvent,
        const ArgList &list
        );
};

class ScriptComponentSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        )
    {
        return false;
    }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    void DeserializeVariables(
        ScriptComponent *pComponent,
        IInputStream *pStream
        );

    virtual ISerializable *Instantiate() const { return new ScriptComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return ScriptComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
