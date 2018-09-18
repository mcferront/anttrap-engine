#include "EnginePch.h"

#include "SplineComponent.h"
#include "Node.h"
#include "SplineAsset.h"
#include "DebugGraphics.h"

DefineComponentType(SplineComponent, new SplineComponentSerializer);

void SplineComponent::Create(
    Id id,
    ResourceHandle spline
    )
{
    SetId( id );
    m_Spline = spline;
}

void SplineComponent::Destroy( void )
{
    m_Spline = NullHandle;

    Component::Destroy( );
}

void SplineComponent::EditorRender( void )
{
    Component::EditorRender( );

    for (int i = 0; i <= 10; i++)
    {
        float p = i / 10.0f;
        Transform transform = GetTransform( p );
        DebugGraphics::Instance( ).RenderTransform( transform, 2.f );
    }
}

void RenderSpline(const BezierSpline *pBezierSpline, const Transform &worldTransform)
{
   if (pBezierSpline->GetNumCurves() <= 0)
      return;

   const int curveSteps = 20;
   
   Transform vertTransform = Math::IdentityTransform( );
   Vector position;
   Vector lastPosition;
   Vector dir;

   for (int i = 0; i < pBezierSpline->GetNumCurves(); i++)
   {
      lastPosition = pBezierSpline->GetPosition(i, 0.0f);
      Math::TransformPosition(&lastPosition, lastPosition, worldTransform);

      for (int t = 1; t <= curveSteps; t++)
      {
         position = pBezierSpline->GetPosition(i, (float)t / curveSteps);
         Math::TransformPosition(&position, position, worldTransform);

         DebugGraphics::Instance().RenderLine(lastPosition, position, Vector(1, 0, 1, 1));
         
         lastPosition = position;
      }
   }

   for (int i = 0; i <= pBezierSpline->GetNumCurves(); i++)
   {
      vertTransform.SetOrientation(Vector(0, 0, 1), Vector(0, 1, 0));
      vertTransform.SetTranslation(pBezierSpline->GetControlPoints()[i * 3]);
      
      Math::Multiply(&vertTransform, vertTransform, worldTransform);

      DebugGraphics::Instance().RenderTransform(vertTransform, 2.0f);
   }
}

Vector SplineComponent::GetClosestPosition(
    const Vector &position
    )
{
    if (false == IsResourceLoaded(m_Spline))    
       return position;

    Debug::Print( Condition(false == GetParent( )->GetWorldTransform().IsScaled()), Debug::TypeError, 
                            "Spline: \'%s\' is scaled, Spline scaling isn't supported.\n", GetParent()->GetId().ToString() );  

    Vector localPosition;
    Vector closestPosition;
    Transform invertWorldMatrix;
    
    Math::Invert(&invertWorldMatrix, GetParent( )->GetWorldTransform());
    Math::TransformPosition(&localPosition, position, invertWorldMatrix);

    Spline *pSpline = GetResource( m_Spline, Spline );
    
    closestPosition = pSpline->GetBezierSpline( )->GetClosestPosition( localPosition );

    Math::TransformPosition(&closestPosition, closestPosition, GetParent( )->GetWorldTransform());

    return closestPosition;
}

Quaternion SplineComponent::GetClosestRotation(
    const Vector &position
    )
{
    if (false == IsResourceLoaded(m_Spline))
       return Math::IdentityQuaternion( );

    Vector localPosition;
    Transform invertWorldMatrix;
    
    Debug::Print( Condition(false == GetParent( )->GetWorldTransform().IsScaled()), Debug::TypeError, 
                            "Spline: \'%s\' is scaled, Spline scaling isn't supported.\n", GetParent()->GetName() );  

    Math::Invert(&invertWorldMatrix, GetParent( )->GetWorldTransform());
    Math::TransformPosition(&localPosition, position, invertWorldMatrix);

    Spline *pSpline = GetResource( m_Spline, Spline );
    
    Quaternion closestRotation = pSpline->GetBezierSpline( )->GetClosestRotation( localPosition );

    closestRotation = closestRotation * GetParent( )->GetWorldTransform().GetOrientation();

    return closestRotation;
}

Transform SplineComponent::GetClosestTransform(
    const Vector &position
    )
{
    if (false == IsResourceLoaded(m_Spline))
       return Transform(Math::IdentityQuaternion( ), position, Vector(1, 1, 1, 1));

    Vector localPosition;
    Transform invertWorldMatrix;
    
    Debug::Print( Condition(false == GetParent( )->GetWorldTransform().IsScaled()), Debug::TypeError, 
                            "Spline: \'%s\' is scaled, Spline scaling isn't supported.\n", GetParent()->GetName() );  

    Math::Invert(&invertWorldMatrix, GetParent( )->GetWorldTransform());
    Math::TransformPosition(&localPosition, position, invertWorldMatrix);

    Spline *pSpline = GetResource( m_Spline, Spline );
    
    Transform closestTransform = pSpline->GetBezierSpline( )->GetClosestTransform( localPosition );

    closestTransform = closestTransform * GetParent( )->GetWorldTransform();

    return closestTransform;
}

float SplineComponent::GetClosestParam(
    const Vector &position
    )
{
    if (false == IsResourceLoaded(m_Spline))
       return 0;

    Vector localPosition;
    Vector closestPosition;
    Transform invertWorldMatrix;
    
    Debug::Print( Condition(false == GetParent( )->GetWorldTransform().IsScaled()), Debug::TypeError, 
                            "Spline %s is scaled, Spline scaling isn't supported.\n", GetParent()->GetName() );  

    Math::Invert(&invertWorldMatrix, GetParent( )->GetWorldTransform());
    Math::TransformPosition(&localPosition, position, invertWorldMatrix);

    Spline *pSpline = GetResource( m_Spline, Spline );
    
    return pSpline->GetBezierSpline( )->GetClosestParam( localPosition );
}

Transform SplineComponent::GetTransform(
    float p
    )
{
    if (false == IsResourceLoaded(m_Spline))
       return Transform(Math::IdentityQuaternion( ), Math::ZeroVector( ), Vector(1, 1, 1));

    Debug::Print( Condition(false == GetParent( )->GetWorldTransform().IsScaled()), Debug::TypeError, 
                            "Spline: \'%s\' is scaled, Spline scaling isn't supported.\n", GetParent()->GetName() );  

    Spline *pSpline = GetResource( m_Spline, Spline );
    
    Transform transform = pSpline->GetBezierSpline( )->GetTransform( p );

    transform = transform * GetParent( )->GetWorldTransform();

    return transform;
}

float SplineComponent::GetLength( void ) const
{
    Spline *pSpline = GetResource( m_Spline, Spline );
    return pSpline->GetBezierSpline( )->GetLength( );
}

ISerializable *SplineComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    if ( NULL == pSerializable ) pSerializable = new SplineComponent; 

    SplineComponent *pSplineComponent = (SplineComponent *) pSerializable;

    Id id = Id::Deserialize( pSerializer->GetInputStream() );
    Id splineId = Id::Deserialize( pSerializer->GetInputStream() );

    pSplineComponent->Create( id, ResourceHandle(splineId) );

    return pSerializable;
}
