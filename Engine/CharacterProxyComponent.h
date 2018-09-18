#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "CharacterProxy.h"

class CharacterProxyComponent : public Component
{
public:
    DeclareComponentType(CharacterProxyComponent);

private:
    CharacterProxy m_CharacterProxy;

public:
    CharacterProxyComponent( void ) {}

    void Create(
        Id id
        );

    void Bind( void );

    void Render( 
        const Vector &color
        );

    void AddToScene( void );
    void RemoveFromScene( void );

    void Destroy( void );

    void UseGravity( 
        bool useGravity
        );

    void ApplyMovement(
        const Vector &delta
        );

    void SetTransform(
        const Transform &transform
        )
    {
        m_CharacterProxy.SetTransform( transform );
    }

    void GetCollisionNormal(
        Vector *pNormal
        ) const
    {
        m_CharacterProxy.GetCollisionNormal( pNormal );
    }
};
