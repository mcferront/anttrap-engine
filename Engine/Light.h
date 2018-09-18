#pragma once

#include "EngineGlobal.h"
#include "RenderObject.h"

class Light : public RenderObject
{
private:
    LightDesc m_Desc;
    Component *m_pComponent;
    IdList m_RenderGroups;
    
    // TODO: move this to a better location
    // For spots
    Vector m_Extents;
    Camera m_Camera;
    Frustum m_Frustum;
    float m_CosOuter;
    float m_CosInner;

public:
    void Create(
        Component *pComponent,
        const IdList &renderGroups,
        const LightDesc &desc
        );

    void Destroy( void );

    virtual bool IsVisible(
        const Frustum &frustum
        ) const;

    void SetColor(
       const Vector &color
    )
    {
       m_Desc.color = color;
    }

    virtual void Flush( void );
    
    virtual bool NeedsFlush( void ) const { return m_Desc.cast == LightDesc::Cast::CastSpot; }

    virtual int GetRenderType( void ) const { return RenderObject::Type::Light; }

    virtual void GetRenderData(
        RendererDesc *pDesc
        ) const;


    virtual void GetWorldTransform(
       Transform *pTransform
       ) const
    {
       *pTransform = m_pComponent->GetParent()->GetWorldTransform();
    }

    virtual void GetRenderGroups( 
        IdList *pGroups
        ) const
    {
        pGroups->CopyFrom( m_RenderGroups );
    }

    virtual bool IsInRange(
       const Transform &world,
       const Transform &invWorld,
       const Vector &center,
       const Vector &extents
    ) const;
};
