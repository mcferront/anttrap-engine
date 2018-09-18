#include "EnginePch.h"

#include "DebugGraphics.h"
#include "Line.h"
#include "Viewport.h"
#include "MeshAsset.h"
#include "MaterialObject.h"
#include "RenderWorld.h"

DebugGraphics &DebugGraphics::Instance( void )
{
   static DebugGraphics s_graphics;
   return s_graphics;
}

DebugGraphics::DebugGraphics( void )
{

}

void DebugGraphics::Create( void )
{
   m_Lines.Create( );
   m_Triangles.Create( );
   m_Spheres.Create( );
   m_Planes.Create( );
   m_MaterialObjectPool.Create( );
   m_UsedMaterialObjects.Create( );

   m_pLinesMaterial = NULL;
   m_pTrianglesMaterial = NULL;  
   
   //m_TextArea.Create( Vector2((float) Mode::Landscape.GetVirtualWidth(), (float) Mode::Landscape.GetVirtualHeight()), NullHandle, 
   //                   ResourceHandle("asset://core/DefaultFont.material"), ResourceHandle("asset://core/DefaultFont.fontmap"), textRendererId ); 
   //m_TextArea.SetVAlign( TextArea::VAlignTop );


   //Transform transform = Math::IdentityTransform;
   //m_TextArea.SetWorldTransform( transform ); 
}

void DebugGraphics::Destroy( void )
{
   //m_TextArea.Destroy( );
   for ( uint32 i = 0; i < m_MaterialObjectPool.GetSize(); i++ )
      delete m_MaterialObjectPool.GetAt(i);

   for ( uint32 i = 0; i < m_UsedMaterialObjects.GetSize(); i++ )
      delete m_UsedMaterialObjects.GetAt(i);

   m_MaterialObjectPool.Destroy( );
   m_UsedMaterialObjects.Destroy( );

   delete m_pLinesMaterial;
   delete m_pTrianglesMaterial;

   m_Lines.Destroy( );
   m_Triangles.Destroy( );
   m_Spheres.Destroy( );
   m_Planes.Destroy( );
}

void DebugGraphics::RenderArrow(
   const Vector &origin,
   const Vector &direction,
   const Vector &color,
   float size
)
{
   Vector top = origin + direction * size;
   Vector bottom = origin;

   Vector leafDirection( direction.y, direction.x, direction.z );

   Vector left = top - ( direction * ( size / 4.0f ) ) + leafDirection * ( size / 4.0f );
   Vector right = top - ( direction * ( size / 4.0f ) ) - leafDirection * ( size / 4.0f );

   RenderLine( Math::IdentityTransform( ), top, bottom, color );
   RenderLine( Math::IdentityTransform( ), left, top, color );
   RenderLine( Math::IdentityTransform( ), right, top, color );
}

void DebugGraphics::RenderTransform(
   const Transform &transform,
   float size
)
{
   Vector right, up, look;
   Vector translation;

   transform.GetRight( &right );
   transform.GetUp( &up );
   transform.GetLook( &look );

   transform.GetTranslation( &translation );

   RenderArrow( translation, right, Vector( 1, 0, 0, 1 ), size );
   RenderArrow( translation, up, Vector( 0, 1, 0, 1 ), size );
   RenderArrow( translation, look, Vector( 0, 0, 1, 1 ), size );
}

void DebugGraphics::RenderLine(
   const Transform &transform,
   const Vector &start,
   const Vector &end,
   const Vector &color
)
{
   Vector s, e;

   Math::TransformPosition( &s, start, transform );
   Math::TransformPosition( &e, end, transform );

   RenderLine( s, e, color );
}

void DebugGraphics::RenderLine(
   const Vector &start,
   const Vector &end,
   const Vector &color
)
{
   Line line;

   line.startColor = color;
   line.endColor = color;

   line.start = start;
   line.end = end;

   m_Lines.Add( line );
}

void DebugGraphics::RenderTriangle(
   const Transform &transform,
   const Vector &v1,
   const Vector &v2,
   const Vector &v3,
   const Vector &color
)
{
   Vector t1, t2, t3;

   Math::TransformPosition( &t1, v1, transform );
   Math::TransformPosition( &t2, v2, transform );
   Math::TransformPosition( &t3, v3, transform );

   RenderTriangle( t1, t2, t3, color );
}

void DebugGraphics::RenderTriangle(
   const Vector &v1,
   const Vector &v2,
   const Vector &v3,
   const Vector &color
)
{
   Triangle t;

   t.vertices[ 0 ].position = v1;
   t.vertices[ 0 ].color = color;
   t.vertices[ 1 ].position = v2;
   t.vertices[ 1 ].color = color;
   t.vertices[ 2 ].position = v3;
   t.vertices[ 2 ].color = color;

   m_Triangles.Add( t );
}

void DebugGraphics::RenderCircle(
   const Vector2 &center,
   float radius,
   const Vector &color
)
{
   Vector2 start;
   Vector2 end;

   int segments = 16;

   for ( int i = 0; i < segments; i++ )
   {
      float c, s;
      Math::CosSin( &c, &s, (float) i / segments * ( Math::PI( ) * 2 ) );

      start.x = c * radius;
      start.y = s * radius;

      start = start + center;

      int next = ( i + 1 ) % segments;
      Math::CosSin( &c, &s, (float) next / segments * ( Math::PI( ) * 2 ) );

      end.x = c * radius;
      end.y = s * radius;

      end = end + center;

      RenderLine( start, end, color );
   }
}

void DebugGraphics::RenderSphere(
   const Transform &transform,
   float radius,
   const Vector &color
)
{
   Sphere sphere;
   sphere.transform = transform;
   sphere.radius = radius;
   sphere.color = color;

   sphere.pMaterial = GetMaterialObject( );

   m_Spheres.Add( sphere );
}

void DebugGraphics::RenderPlane(
   const Vector &p,
   const Vector &color
)
{
   Plane plane;

   plane.plane = p;
   plane.color = color;

   m_Planes.Add( plane );
}

void DebugGraphics::RenderBox(
   const Transform &transform,
   const Vector2 &center,
   const Vector2 &half,
   const Vector &color
)
{
   Vector2 topRight( 1, 1 );
   Vector2 topLeft( -1, 1 );
   Vector2 bottomRight( 1, -1 );
   Vector2 bottomLeft( -1, -1 );

   RenderLine( transform, topLeft * half + center, topRight * half + center, color );
   RenderLine( transform, bottomLeft * half + center, bottomRight * half + center, color );

   RenderLine( transform, topLeft * half + center, bottomLeft * half + center, color );
   RenderLine( transform, topRight * half + center, bottomRight * half + center, color );
}

void DebugGraphics::RenderCube(
   const Transform &transform,
   const Vector &center,
   const Vector &half,
   const Vector &color
)
{
   Vector top1( -1, 1,  1 );
   Vector top2( -1, 1, -1 );
   Vector top3( 1,  1, -1 );
   Vector top4( 1,  1,  1 );

   Vector bottom1( -1, -1,  1 );
   Vector bottom2( -1, -1, -1 );
   Vector bottom3(  1, -1, -1 );
   Vector bottom4(  1, -1,  1 );

   RenderLine( transform, top1 * half + center, top2 * half + center, color );
   RenderLine( transform, top2 * half + center, top3 * half + center, color );
   RenderLine( transform, top3 * half + center, top4 * half + center, color );
   RenderLine( transform, top4 * half + center, top1 * half + center, color );

   RenderLine( transform, bottom1 * half + center, bottom2 * half + center, color );
   RenderLine( transform, bottom2 * half + center, bottom3 * half + center, color );
   RenderLine( transform, bottom3 * half + center, bottom4 * half + center, color );
   RenderLine( transform, bottom4 * half + center, bottom1 * half + center, color );

   RenderLine( transform, top1 * half + center, bottom1 * half + center, color );
   RenderLine( transform, top2 * half + center, bottom2 * half + center, color );
   RenderLine( transform, top3 * half + center, bottom3 * half + center, color );
   RenderLine( transform, top4 * half + center, bottom4 * half + center, color );
}

void DebugGraphics::RenderFrustum(
   const Frustum &frustum,
   const Vector &color
)
{
   Debug::Assert( Condition(false), "Not Implemented" );
}

void DebugGraphics::Print(
   const char *pText,
   ...
)
{
   va_list args;

   va_start( args, pText );

   char message[ 256 ];
   String::FormatV( message, sizeof( message ), pText, args );

   //m_TextArea.Print( message );

   va_end( args );
}

void DebugGraphics::Clear( void )
{
   //m_TextArea.Clear( );
   m_Lines.Clear( );
   m_Triangles.Clear( );
   m_Spheres.Clear( );
   m_Planes.Clear( );
}

void DebugGraphics::AddToScene( void )
{  
   static const char *pMaterial = StringRef( "DebugGraphics.material" );

   if ( NULL == m_pLinesMaterial )
      m_pLinesMaterial = new GraphicsMaterialObject( ResourceHandle::FromAlias( pMaterial ) );

   if ( NULL == m_pTrianglesMaterial )
      m_pTrianglesMaterial = new GraphicsMaterialObject( ResourceHandle::FromAlias( pMaterial ) );   RenderObject::AddToScene( );

   //m_TextArea.AddToScene( );
}

void DebugGraphics::RemoveFromScene( void )
{
   //m_TextArea.RemoveFromScene( );

   RenderObject::RemoveFromScene( );
}

void DebugGraphics::GetRenderData(
   RendererDesc *pDesc
) const
{
   if ( m_Lines.GetSize( ) )
   {
      if ( true == m_pLinesMaterial->Prepare( ) && true == m_pLinesMaterial->HasPass(pDesc->pPass) )
      {
         RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

         uint32 count = m_Lines.GetSize( );

         Line *pLines = new Line[ count ];
         m_Lines.CopyTo( pLines, count );

         pRODesc->renderContext = m_pLinesMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), Line::GetVertexContext( ) );
         pRODesc->pMaterial = m_pLinesMaterial;
         pRODesc->renderFunc = DebugGraphics::RenderLines;
         pRODesc->pObject = this;
         pRODesc->argList = ArgList( count );
         pRODesc->buffer.Write( pLines, sizeof( Line ) * count );

         pDesc->renderObjectDescs.Add( pRODesc );

         delete[ ] pLines;
      }
   }

   if ( m_Triangles.GetSize( ) )
   {
      if ( true == m_pTrianglesMaterial->Prepare( ) && true == m_pTrianglesMaterial->HasPass(pDesc->pPass) )
      {
         RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

         uint32 count = m_Triangles.GetSize( );

         Triangle *pTriangles = new Triangle[ count ];
         m_Triangles.CopyTo( pTriangles, count );

         pRODesc->renderContext = m_pTrianglesMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), Triangle::GetVertexContext() );
         pRODesc->pMaterial = m_pTrianglesMaterial;
         pRODesc->renderFunc = DebugGraphics::RenderTriangles;
         pRODesc->pObject = this;
         pRODesc->argList = ArgList( count );
         pRODesc->buffer.Write( pTriangles, sizeof( Triangle ) * count );

         pDesc->renderObjectDescs.Add( pRODesc );

         delete[ ] pTriangles;
      }
   }

   if ( m_Spheres.GetSize( ) )
   {
      if ( true == m_Spheres.GetAt(0).pMaterial->Prepare() && true == m_Spheres.GetAt(0).pMaterial->HasPass(pDesc->pPass) )
      {
         Mesh *pModel = GetResource( ResourceHandle::FromAlias( "Unit_Sphere.mesh" ), Mesh );

         for ( uint32 i = 0; i < m_Spheres.GetSize(); i++ )
         {
            RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

            Sphere *pSphere = m_Spheres.GetPointer(i);

            pSphere->pMaterial->Prepare( );

            pRODesc->renderContext = pSphere->pMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), pModel->GetSurface( 0 )->GetVertexContext() );
            pRODesc->pMaterial = pSphere->pMaterial;
            pRODesc->renderFunc = DebugGraphics::RenderSphere;
            pRODesc->pObject = this;
            pRODesc->argList = ArgList( pSphere->radius, pSphere->color, pSphere->transform );

            pDesc->renderObjectDescs.Add( pRODesc );
         }
      }
   }

   if ( m_Planes.GetSize( ) )
   {
      Debug::Print( Debug::TypeWarning, "DebugGraphics::RenderPlanes is not yet implemented\n" );

      //RenderDesc desc;

      //uint32 count = m_Planes.GetSize();

      //Plane *pPlanes = new Plane[ count ];
      //m_Planes.CopyTo( pPlanes, count );

      //desc.material   = ResourceHandle(pMaterial);
      //desc.renderFunc = DebugGraphics::RenderPlanes;
      //desc.pObject    = this;
      //desc.argList    = ArgList( count );
      //desc.buffer.Write( pPlanes, sizeof(Plane) * count );

      //pList->Add( desc );

      //delete []pPlanes;
   }
}

GraphicsMaterialObject *DebugGraphics::GetMaterialObject( void )
{
   if ( 0 == m_MaterialObjectPool.GetSize() )
   {
      static const char *pMaterial = StringRef( "DebugGraphics.material" );
      m_MaterialObjectPool.Add( new GraphicsMaterialObject(ResourceHandle::FromAlias(pMaterial)) );
   }

   GraphicsMaterialObject *pObject = m_MaterialObjectPool.RemoveAt( 0 );

   m_UsedMaterialObjects.Add( pObject );

   return pObject;
}

// TODO: it's ok to do this before the renderer finishes
// because by the time new data is sent to them it'll be rendering the next frame
void DebugGraphics::FreeMaterialObjects( void )
{
   while ( m_UsedMaterialObjects.GetSize() )
   {
      GraphicsMaterialObject *pObject = m_UsedMaterialObjects.RemoveAt( 0 );
      m_MaterialObjectPool.Add( pObject );
   }
}

void DebugGraphics::RenderLines(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   uint32 numLines;
   desc.pDesc->argList.GetArg( 0, &numLines );

   Line *pLines = (Line *) desc.pDesc->buffer.Read( 0 );

   Line::Render( pLines, numLines, false, desc.pDesc->pMaterial, Math::IdentityTransform( ), desc, pStats );
}

void DebugGraphics::RenderTriangles(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   uint32 numTriangles;
   desc.pDesc->argList.GetArg( 0, &numTriangles );

   Triangle *pTriangles = (Triangle *) desc.pDesc->buffer.Read( 0 );

   Triangle::Render( pTriangles, numTriangles, false, desc.pDesc->pMaterial, Math::IdentityTransform( ), desc, pStats );
}

void DebugGraphics::RenderSphere(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   float radius;
   Vector color;
   Transform transform;

   desc.pDesc->argList.GetArg( 0, &radius );
   desc.pDesc->argList.GetArg( 1, &color );
   desc.pDesc->argList.GetArg( 2, &transform );

   Mesh     *pModel = GetResource( ResourceHandle::FromAlias( "Unit_Sphere.mesh" ), Mesh );

   Transform viewTransform;
   desc.pViewport->GetCamera( )->GetViewTransform( &viewTransform );

   Matrix projection;
   desc.pViewport->GetCamera( )->GetProjection( &projection );
   //Math::Multiply( &projection, projection, desc.pViewport->GetMode( )->GetRotation( ) );

   static const char *pWorld = StringRef( "$WORLD_MATRIX" );
   static const char *pVP = StringRef( "$VP_MATRIX" );
   static const char *pColor = StringRef( "$COLOR" );

   Matrix vp;
   Math::Multiply( &vp, viewTransform.ToMatrix(true), projection );

   GraphicsMaterialObject *pMaterial = desc.pDesc->pMaterial;
   GraphicsMaterialObject::Pass *pPass = pMaterial->GetPass( desc.pDesc->renderContext );
   
   if ( NULL == pPass )
      return;

   int worldIndex = pPass->GetData()->GetMatrixMacroIndex( pWorld );
   int colorIndex = pPass->GetData()->GetVectorMacroIndex( pColor );

   pPass->GetData()->SetMacro( pVP, &vp, 1 );

   transform.SetScale( Vector( radius * 2, radius * 2, radius * 2, 1 ) );

   pPass->GetData()->SetMacro( colorIndex, &color, 1 );
   pPass->GetData()->SetMacro( worldIndex, &transform.ToMatrix( true ), 1 );

   pMaterial->SetRenderData( pPass, desc.pCommandList );

   const Surface *pSurface = pModel->GetSurface( 0 );
   pSurface->Render( desc.pCommandList, pStats );
}

void DebugGraphics::RenderPlanes(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   *pStats = RenderStats::Empty;

   //uint32 numPlanes;
   //desc.argList.GetArg( 0, &numPlanes );

   //Material *pMaterial = GetResource( desc.material, Material );
   //Plane   *pPlanes  = (Plane *) desc.buffer.Read( 0 );

   //for (uint32 i = 0; i < numPlanes; i++ )
   //{
   //}
}
