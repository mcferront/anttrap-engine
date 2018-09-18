#include "EnginePch.h"

#include "Geometry.h"

void CalculateNormals( 
   Triangle *pTriangles, 
   uint32 numTriangles 
)
{
   uint32 i;
   
   for ( i = 0; i < numTriangles; i++ )
   {
      Vector c0 = pTriangles[ i ].vertices[ 1 ].position - pTriangles[ i ].vertices[ 0 ].position;
      Vector c1 = pTriangles[ i ].vertices[ 2 ].position - pTriangles[ i ].vertices[ 0 ].position;
      
      Vector cross;

      //Math::Normalize( &c0, c0 );
      //Math::Normalize( &c1, c1 );

      Math::CrossProduct( &cross, c0, c1 );
      Math::Normalize( &cross, cross );

      pTriangles[ i ].vertices[ 0 ].normal = cross;
      pTriangles[ i ].vertices[ 1 ].normal = cross;
      pTriangles[ i ].vertices[ 2 ].normal = cross;
   }
}
/*
bool Capsule::TestCollision(
   const Capsule &capsuleB,
   CollisionDesc *pDesc
) const
{
   Vector2 delta = m_Point - capsuleB.m_Point;

   float fa00	=   Math::MagnitudeSq( m_Direction );
	float fa01	= - Math::DotProduct ( m_Direction, capsuleB.m_Direction );
	float fa11	=   Math::MagnitudeSq( capsuleB.m_Direction );
	float fb0	=   Math::DotProduct ( delta, m_Direction );
	float fc	   =   Math::MagnitudeSq( delta );
	
   float fdet	= Math::Abs( fa00 * fa11 - fa01 * fa01 );

	float fs, ft, ftmp, sqrdist;

	if ( fdet > 0.0f )
	{
      float fb1 = - Math::DotProduct( delta, capsuleB.m_Direction );

		fs = fa01 * fb1 - fa11 * fb0;

		ft = fa01 * fb0 - fa00 * fb1;

		if ( fs >= 0.0f )
		{
			if ( fs <= fdet )
			{
				if ( ft >= 0.0f )
				{
					if ( ft <= fdet )
					{
						float invdet = 1.0f / fdet;

						fs *= invdet;
						ft *= invdet;

						sqrdist = fs * ( fa00 * fs + fa01 * ft + 2.0f * fb0 ) + ft * ( fa01 * fs + fa11 * ft + 2.0f * fb1 ) + fc;               
					}
					else
					{
						ft = 1.0f;

						ftmp = fa01 + fb0;

						if ( ftmp >= 0.0f )
						{
							fs = 0.0f;
                     sqrdist = fa11 + 2.0f * fb1 + fc;
						}
						else if ( - ftmp >= fa00 )
						{
							fs = 1.0f;
							sqrdist = fa00 + fa11 + fc + 2.0f * ( fb1 + ftmp );
						}
						else
						{
							fs = - ftmp * ( 1.0f / fa00 );
							sqrdist = ftmp * fs + fa11 + 2.0f * fb1 + fc;
						}
					}
				}
				else
				{
					ft = 0.0f;

					if ( fb0 >= 0.0f )
					{
						fs = 0.0f;
						sqrdist = fc;
					}
					else if ( - fb0 >= fa00 )
					{
						fs = 1.0f;
						sqrdist = fa00 + 2.0f * fb0 + fc;

					}
					else 
					{
						fs = - fb0 * ( 1.0f / fa00 );
						sqrdist = fb0 * fs + fc;
					}
				}
			}
			else
			{
				if ( ft >= 0.0f )
				{
					if ( ft <= fdet )
					{
						fs = 1.0f;

						ftmp = fa01 + fb1;

						if ( ftmp >= 0.0f )
						{
							ft = 0.0f;
							sqrdist = fa00 + 2.0f * fb0 + fc;
						}
						else if ( - ftmp >= fa11 )
						{
							ft = 1.0f;
							sqrdist = fa00 + fa11 + fc + 2.0f * ( fb0 + ftmp );
						}
						else
						{
							ft = - ftmp * ( 1.0f / fa11 );
							sqrdist = ftmp * ft + fa00 + 2.0f * fb0 + fc;
						}
					}
					else
					{
						ftmp = fa01 + fb0;

						if ( - ftmp <= fa00 )
						{
							ft = 1.0f;

							if ( ftmp >= 0.0f )
							{
								fs = 0.0f;
                        sqrdist = fa11 + 2.0f * fb1 + fc;
							}
							else
							{
								fs = - ftmp * ( 1.0f / fa00 );
								sqrdist = ftmp * fs + fa11 + 2.0f * fb1 + fc;
							}
						}
						else
						{
							fs = 1.0f;
							ftmp = fa01 + fb1;

							if ( ftmp >= 0.0f )
							{
								ft = 0.0f;
								sqrdist = fa00 + 2.0f * fb0 + fc;
							}
							else if ( - ftmp >= fa11 )
							{
								ft = 1.0f;
                        sqrdist = fa00 + fa11 + fc + 2.0f * ( fb0 + ftmp );
							}
							else
							{
								ft = - ftmp * ( 1.0f / fa11 );
								sqrdist = ftmp * ft + fa00 + 2.0f * fb0 + fc;
							}
						}
					}
				}
				else
            {
				   if ( -fb0 < fa00 )
					{
						ft = 0.0f;

						if ( fb0 >= 0.0f )
                  {
                     fs = 0.0f;
							sqrdist = fc;
                  }
                  else
                  {
                     fs = - fb0 * ( 1.0f / fa00 );
                     sqrdist = fb0 * fs + fc;
                  }
               }
               else
               {
                  fs = 1.0f;
                        
						ftmp = fa01 + fb1;
                        
						if ( ftmp >= 0.0f )
                  {
                     ft = 0.0f;
							sqrdist = fa00 + 2.0f * fb0 + fc;
                  }
                  else if ( - ftmp >= fa11 )
                  {
                     ft = 1.0f;
							sqrdist = fa00 + fa11 + fc + 2.0f * ( fb0 + ftmp );
                  }
                  else
                  {
                     ft = - ftmp * ( 1.0f / fa11 );
							sqrdist = ftmp * ft + fa00 + 2.0f * fb0 + fc;
                  }
               }
            }
         }
      }
      else 
      {
         if ( ft >= 0.0f )
         {
            if ( ft <= fdet )  // region 5 (side)
            {
               fs = 0.0f;

					if ( fb1 >= 0.0f )
               {
                  ft = 0.0f;
						sqrdist = fc;
               }
               else if ( - fb1 >= fa11 )
               {
                  ft = 1.0f;
						sqrdist = fa11 + 2.0f * fb1 + fc;
               }
               else
               {
                  ft = - fb1 * ( 1.0f / fa11 );
						sqrdist= fb1 * ft + fc;
               }
            }
            else  // region 4 (corner)
            {
               ftmp = fa01 + fb0;
                    
					if ( ftmp < 0.0f )
               {
                  ft = 1.0f;
                        
						if ( - ftmp >= fa00 )
                  {
                     fs = 1.0f;
							sqrdist = fa00 + fa11 + fc + 2.0f * ( fb1 + ftmp );
                  }
                  else
                  {
                     fs = - ftmp * ( 1.0f / fa00 );
							sqrdist = ftmp * fs + fa11 + 2.0f * fb1 + fc;
                  }
               }
               else
               {
                  fs = 0.0f;
      
                  if ( fb1 >= 0.0f )
                  {
                     ft = 0.0f;
							sqrdist = fc;
                  }
                  else if ( - fb1 >= fa11 )
                  {
                     ft = 1.0f;
							sqrdist = fa11 + 2.0f * fb1 + fc;
                  }
                  else
                  {
                     ft = - fb1 * ( 1.0f / fa11 );
							sqrdist = fb1 * ft + fc;
                  }
               }
            }
         }
         else   // region 6 (corner)
         {
            if ( fb0 < 0.0f )
            {
               ft = 0.0f;
                    
					if ( - fb0 >= fa00 )
               {
                  fs = 1.0f;
						sqrdist = fa00 + 2.0f * fb0 + fc;
               }
               else
               {
                  fs = - fb0 * ( 1.0f / fa00 );
						sqrdist = fb0 * fs + fc;
               }
            }
            else
            {
               fs = 0.0f;
                    
					if ( fb1 >= 0.0f )
               {
                  ft = 0.0f;
						sqrdist = fc;
               }
               else if ( - fb1 >= fa11 )
               {
                  ft = 1.0f;
						sqrdist = fa11 + 2.0f * fb1 + fc;
               }
               else
               {
                  ft = - fb1 * ( 1.0f / fa11 );
						sqrdist = fb1 * ft + fc;
               }
            }
         }
      }
   }
   else
   {
      // line segments are parallel
   	if ( fa01 > 0.0f )
      {
         // direction vectors form an obtuse angle
         if ( fb0 >= 0.0f )
         {
            fs = 0.0f;
			   ft = 0.0f;
			   sqrdist = fc;
         }
         else if ( - fb0 <= fa00 )
         {
            fs = - fb0 * ( 1.0f / fa00 );
            ft = 0.0f;
   			sqrdist = fb0 * fc + fc;
         }
         else
         {
            float fb1 = - Math::DotProduct( delta, capsuleB.m_Direction );
               
			   fs = 1.0f;
                  
			   ftmp = fa00 + fb0;
               
			   if ( -ftmp >= fa01 )
            {
               ft = 1.0f;
   				sqrdist = fa00 + fa11 + fc + 2.0f * ( fa01 + fb0 + fb1 );
            }
            else
            {
               ft = - ftmp * ( 1.0f / fa01 );
	            sqrdist = fa00 + 2.0f * fb0 + fc + ft * ( fa11 * ft + 2.0f * ( fa01 + fb1 ) );
            }
         }
      }
      else
      {
         // direction vectors form an acute angle
      	if ( - fb0 >= fa00 )
         {
            fs = 1.0f;
            ft = 0.0f;
				sqrdist = fa00 + 2.0f * fb0 + fc;
         }
         else if ( fb0 <= 0.0f )
         {
            fs = - fb0 * ( 1.0f / fa00 );
			   ft = 0.0f;
			   sqrdist = fb0 * fs + fc;
         }
         else
         {
            float fb1 = - Math::DotProduct( delta, capsuleB.m_Direction );

			   fs = 0.0f;
               
   			if ( fb0 >= - fa01 )
            {
               ft = 1.0f;
				   sqrdist = fa11 + 2.0f * fb1 + fc;
            }
            else
            {
               ft = - fb0 * ( 1.0f / fa01 );
   				sqrdist = fc + ft * ( 2.0f * fb1 + fa11 * ft );
            }
         }
      }
   }

   Vector2 nearestA, nearestB;

   nearestA = m_Point + m_Direction * fs;
   nearestB = capsuleB.m_Point + capsuleB.m_Direction * ft;

   delta = nearestB - nearestA;

   float distanceSq     = Math::Abs( sqrdist );
   float combinedRadius = m_Radius + capsuleB.m_Radius;

   if ( distanceSq <= combinedRadius * combinedRadius )
   {
      float distance    = Math::Sqrt( distanceSq );
	   float invDistance = 1.0f / distance;
	
      pDesc->penetration = combinedRadius - distance;

      pDesc->normal = - delta * invDistance;

	   pDesc->intersection = pDesc->normal * m_Radius;
      pDesc->intersection += nearestA;

      return true;
   }
   
   return false;
}
   
bool Capsule::TestCollision(
   const Plane &plane,
   CollisionDesc *pDesc
) const
{
   Vector start;
   
   start.x = m_Point.x;
   start.y = m_Point.y;
   start.z = 0;

   float distance = Math::DotProduct(start, plane.vector);
   distance += plane.vector.w;

   if ( distance < m_Radius )
   {
      pDesc->penetration = m_Radius - distance;
      pDesc->normal.x = plane.vector.x;
      pDesc->normal.y = plane.vector.y;

	   pDesc->intersection = - pDesc->normal * m_Radius;
      pDesc->intersection += m_Point;

      return true;
   }
   
   Vector2 point = m_Point + m_Direction;

   start.x = point.x;
   start.y = point.y;
   start.z = 0;

   distance = Math::DotProduct(start, plane.vector);
   distance += plane.vector.w;

   if ( distance < m_Radius )
   {
      pDesc->penetration = m_Radius - distance;
      pDesc->normal.x = plane.vector.x;
      pDesc->normal.y = plane.vector.y;

	   pDesc->intersection = - pDesc->normal * m_Radius;
      pDesc->intersection += point;

      return true;
   }

   return false;
}*/