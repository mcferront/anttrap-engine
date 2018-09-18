#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "VertexBuffer.h"

#if defined WIN32
   extern PFNGLBINDATTRIBLOCATIONPROC  pglBindAttribLocation;
   extern PFNGLLINKPROGRAMPROC         pglLinkProgram       ;
   extern PFNGLATTACHSHADERPROC        pglAttachShader      ;
   extern PFNGLCOMPILESHADERPROC       pglCompileShader     ;
   extern PFNGLSHADERSOURCEPROC        pglShaderSource      ;
   extern PFNGLCREATESHADERPROC        pglCreateShader      ;
   extern PFNGLCREATEPROGRAMPROC       pglCreateProgram     ;
   extern PFNGLUSEPROGRAMPROC          pglUseProgram        ;
   extern PFNGLUNIFORMMATRIX4FVPROC    pglUniformMatrix4fv  ;
   extern PFNGLUNIFORM1IPROC           pglUniform1i         ;
   extern PFNGLGETUNIFORMLOCATIONPROC  pglGetUniformLocation;
   extern PFNGLACTIVETEXTUREPROC       pglActiveTexture     ;
   extern PFNGLUNIFORM4FPROC           pglUniform4f         ;
   extern PFNGLVERTEXATTRIBPOINTERPROC pglVertexAttribPointer;
   extern PFNGLDELETEBUFFERSPROC       pglDeleteBuffers     ;
   extern PFNGLBINDBUFFERPROC          pglBindBuffer        ;
   extern PFNGLGENBUFFERSPROC          pglGenBuffers        ;
   extern PFNGLBUFFERDATAPROC          pglBufferData        ;
   extern PFNGLGETSHADERINFOLOGPROC    pglGetShaderInfoLog  ;
   extern PFNGLGETPROGRAMINFOLOGPROC   pglGetProgramInfoLog ;
   extern PFNGLISSHADERPROC            pglIsShader          ;
   extern PFNGLGETPROGRAMIVPROC        pglGetProgramiv      ;
   extern PFNGLGETATTRIBLOCATIONPROC   pglGetAttribLocation ;
   extern PFNGLDELETESHADERPROC        pglDeleteShader      ;

   extern PFNGLENABLEVERTEXATTRIBARRAYPROC  pglEnableVertexAttribArray;
   extern PFNGLDISABLEVERTEXATTRIBARRAYPROC pglDisableVertexAttribArray;

#elif defined IOS || defined MAC || defined ANDROID
   #define pglBindAttribLocation glBindAttribLocation
   #define pglLinkProgram glLinkProgram        
   #define pglAttachShader glAttachShader       
   #define pglCompileShader glCompileShader      
   #define pglShaderSource glShaderSource      
   #define pglCreateShader glCreateShader 
   #define pglCreateProgram glCreateProgram 
   #define pglUseProgram glUseProgram 
   #define pglUniformMatrix4fv glUniformMatrix4fv 
   #define pglUniform1i glUniform1i 
   #define pglGetUniformLocation glGetUniformLocation
   #define pglActiveTexture glActiveTexture 
   #define pglUniform4f glUniform4f 
   #define pglVertexAttribPointer glVertexAttribPointer 
   #define pglDeleteBuffers glDeleteBuffers 
   #define pglBindBuffer glBindBuffer 
   #define pglGenBuffers glGenBuffers 
   #define pglBufferData glBufferData 
   #define pglGetShaderInfoLog glGetShaderInfoLog 
   #define pglGetProgramInfoLog glGetProgramInfoLog 
   #define pglIsShader glIsShader 
   #define pglGetProgramiv glGetProgramiv
   #define pglGetAttribLocation glGetAttribLocation
   #define pglDeleteShader glDeleteShader 
   #define pglEnableVertexAttribArray glEnableVertexAttribArray
   #define pglDisableVertexAttribArray glDisableVertexAttribArray 
#else
   #error Platform undefined
#endif  


#define glCheckError(x) CheckError((x),0,__FILE__,__LINE__)
#define glCheckErrorParam(x,o) CheckError((x),o,__FILE__,__LINE__)

void CheckError(
   const char *pName,
   GLuint obj,
   const char *pFile,
   int line
);

class Shader : public Asset
{
public:
   DeclareResourceType(Shader);

   uint32 m_SkinnedShader;
   uint32 m_StaticShader;
   uint32 m_StaticSelectedShader;
   uint32 m_SkinnedSelectedShader;
   uint32 m_ActiveShader;
   uint32 m_NumTextures;

   HashTable<const char *, int> m_StaticLocations;
   HashTable<const char *, int> m_SkinnedSelectedLocations;
   HashTable<const char *, int> m_SkinnedLocations;
   HashTable<const char *, int> m_StaticSelectedLocations;
   bool   m_NeedsCreate;

public:
   void Create(
      const Cwid &id
   );

   void Destroy( void );

   void MakeActive( bool selected, bool skinned );

   void SetTransforms(
      const char *pName,
      const Transform *pTransforms,
      int numTransforms
   );

   void SetMatrices(
      const char *pName,
      const Matrix *pMatrices,
      int numMatrices
   );

   void SetTexture(
      int sampler,
      ResourceHandle texture,
      bool wrap,
      bool filter
   );

   void SetVector(
      const char *pName,
      const Vector &vector
   );
   
   void SetLight(
      int lightIndex,
      const LightDesc &desc
   );

private:
   int GetUniformLocation(
      const char *pName
   );
 
   void CreateGl ( void );
   void DestroyGl( void );
};

class Gl
{
private:
   Channel *m_pChannel;

public:
   static Gl &Instance( void );

   void CreateChannel ( void );
   void DestroyChannel( void );

   Channel *GetChannel( void ) { return m_pChannel; }
};