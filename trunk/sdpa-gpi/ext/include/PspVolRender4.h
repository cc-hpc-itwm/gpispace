//PspSRender.h

#ifndef	psp_vol_render_H
#define psp_vol_render_H


#include "Datastructs.h"
#include "PspMathUtil.h"
#include "PspCamera.h"

#include <MCTP2.h>
#include <float.h>
#include <vector>

using namespace std;

class tiledata;


class psp_vol_render{

public:

  psp_vol_render(psp_camera *c,mctp2 *tp);
	~psp_vol_render();

  int loadSurfaceDirect(const kdtHeader header,unsigned int *id,triangle *obj,kdtNodePtr nodes,primitiveH *pH,
                        primitiveV *pV,float *vD);

  int loadVolDirect(const int volNr,volumeHeader h,offset64Ptr o64,kdtNodePtr nag,kdtNodePtr *na64);
	void setImage(unsigned char *p,int width,int height);
  int cursorTracking(const float sx,const float sy,float &xpos,float &ypos,float &zpos);
  void renderScene(const int tID);
  void setColorTable(floatRGB lut[256]);

  float getVersion();
  int getTextureWidth();
  int getTextureHeight();
  unsigned int *getTexturePtr();

  float shiftX,shiftY,shiftZ;
  float swap;
  unsigned short xdim,ydim,zdim;
  psp_vec3f mins,maxs;
  unsigned int leaveCount;
  unsigned int totalCount;
  unsigned int objCount;
  unsigned int idCount;
  rgbColor bgColor;

  triangle *triObjects;
  unsigned int *idArray;
  primitiveH *pArrayH;
  primitiveV *pArrayV;
  float *veloData;
  int formatTyp;

  floatRGB clut[256];
  float origBounds[6];

  kdtNodePtr root;
  unsigned long addrBaseNew;

  cleanTiles cT[32];
  volObject volobj[2];
  float *dBuffer;

  void sliderMinMaxX(const unsigned int volNr,const int minV,const int maxV);
  void sliderMinMaxY(const unsigned int volNr,const int minV,const int maxV);
  void sliderMinMaxZ(const unsigned int volNr,const int minV,const int maxV);

  void sliderSfMinMaxX(const int minV,const int maxV);
  void sliderSfMinMaxY(const int minV,const int maxV);
  void sliderSfMinMaxZ(const int minV,const int maxV);

  Color colorTableV0[256];
  Color colorTableV1[256];

  bool adjustTree;
  bool calcDepthBuffer;

  void setLeaveThres(const unsigned int thres){ leaveThres = thres; }
  void renderTexture(const bool b0){ showTexture = b0; }
  void setTextureDims(const int w,const int h){ texW=w; texH=h; }
  void setSemiVolume(const unsigned int vNr){ semiVolNr = vNr; }
  void renderHorizon(const bool b0);

  void setupDepthBuffer();

  int setVoxelScaleOpaqueVol(psp_vec3f s);
  int setVoxelScaleSemiVol(psp_vec3f s);

  volatile unsigned int totalSubTiles;
  volatile unsigned int mainIndex;
  mainTilesPtr stPtr;
  unsigned char *img;
  psp_camera *cam;

private:

  void rebuildTree(const int tID);
  void resetTree(const int tID);

  int leaveThres;//default: 500
  int createTexture(const int width,const int height);

  int cursorTrackingSemi(const float x,const float y,float &tv0);
  int cursorTrackingOpa(const float x,const float y,float &tv1);

  void renderVolume(const int subID,tiledata &td);
  void renderVolumeOpaque(const int subID,tiledata &td);

  void renderOmniSurfaceNC(const int sx,const int sy,const int fx,const int fy,const int fz,
                           __m128 &lmin128,__m128 &lmax128,const __m128 *idir128,const __m128 *odir128,tiledata &td,int &tpos);

  kdtHeader header;
	kdtNodePtr nodeArray,nodeArrayTmp;

  __m128 *min128,*max128;
  psp_vec3f tbMin,tbMax;

  //mctp2
  mctp2 *tp;

  void calcDepth(const psp_vec3f v,float* winz);
  void calcDepth128(const __m128 *pos128,__m128 &z128);

  float depthMatrix[16];
  int viewport[4];

  psp_vec3f scaleVOpa,inv_scaleVOpa;
  psp_vec3f scaleVSemi,inv_scaleVSemi;

  texture internalTexture;

	int ww,wh;
  int texW,texH;
  unsigned int semiVolNr;
  bool showTexture;
  bool showHor;


};


class KdtCompiler;

class triID{
public:

triID(unsigned int i0,unsigned int i1,unsigned int i2){
  id0=i0;id1=i1;id2=i2;
}

unsigned int id0,id1,id2;
};

typedef std::vector<psp_vec3f> floatVec3f;
typedef std::vector<triID> triIDVec;


class horizonComp{

public:

  horizonComp(mctp2 *tp);
  int buildTriangles(const point3D *points,const unsigned int count);
  int setTriangles(const simpleFace *sf,const unsigned int count);
  int buildHorizon(horizonData &hd);
  int buildTree(const int tID);
  void freeMem();
  float getVersion();

  int textureArea(const int minU,const int maxU,const int minV,const int maxV);

private:

  void loadXYZ(const point3D *points,const unsigned int count);
  KdtCompiler *kdtCom;

  unsigned int tID;

  unsigned int elemCnt;
  primitive *pArray;
  primitiveH *pArrayH;
  primitiveV *pArrayV;

  floatVec3f lv;
  triIDVec lt;

  float bb[6];
  float inv_range;
  psp_vec3f worldMin,worldMax;

  unsigned int pCount;
  unsigned int vCount,lutSize;
  float *veloData;
  float minVelo,maxVelo;
  float headerMinTime,headerDeltaT;


};


#endif