/*
This is one of the oldest songs.  I picked
the random name "risen" and said to God
"Oh, you're ambitious," thinking it was
an epic name.  He laughed and gave an
epic song!
*/

F64 t0=Beat;

#define WING	6
#define BODY	2

U0 Bird(CDC *dc,F64 wing_�)
{
  CD3I32 p[3];
  dc->color=WHITE;

  p[0].x=0; p[0].y=0; p[0].z=-BODY;
  p[1].x=0; p[1].y=0; p[1].z=BODY;
  p[2].x=-WING*Cos(wing_�); p[2].y=-WING*Sin(wing_�); p[2].z=0;
  GrFillPoly3(dc,3,p);

  p[0].x=0; p[0].y=0; p[0].z=-BODY;
  p[1].x=0; p[1].y=0; p[1].z=BODY;
  p[2].x=WING*Cos(wing_�); p[2].y=-WING*Sin(wing_�); p[2].z=0;
  GrFillPoly3(dc,3,p);

  dc->color=ROPF_DITHER+WHITE<<16+LTGRAY;
  GrLine3(dc,0,0,-BODY,0,0,BODY);
}

#define BIRDS_NUM	7
#define RANGE		5

F64 bx[BIRDS_NUM],by[BIRDS_NUM];

U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 �,dt=Beat-t0;
  DCDepthBufAlloc(dc);
  dc->x=325;
  dc->y=95;
  dc->z=GR_Z_ALL;
  dc->flags|=DCF_TRANSFORMATION;
  for (i=0;i<BIRDS_NUM;i++) {
    bx[i]=Clamp(bx[i]+0.25*SignI64(RandI16),-RANGE,RANGE);
    by[i]=Clamp(by[i]+0.25*SignI64(RandI16),-RANGE,RANGE);
    �=�/2*i/BIRDS_NUM+0.2*�*dt+�/2;
    Mat4x4IdentEqu(dc->r);
    Mat4x4RotX(dc->r,�/2);
    Mat4x4RotZ(dc->r,�);
    Mat4x4TranslationEqu(dc->r,50*Cos(�)+bx[i],50*Sin(�)+by[i],
	  30*i/BIRDS_NUM+4*dt);
    Mat4x4RotX(dc->r,0.35*�);
    Bird(dc,Sin(2*�*dt+i*2*�/BIRDS_NUM));
  }
}

U0 Song()
{
  I64 i;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=LTCYAN<<4+BLACK;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  for (i=0;i<BIRDS_NUM;i++) {
    bx[i]=RANGE*RandI16/RANGE;
    by[i]=RANGE*RandI16/RANGE;
  }
  try {
    "$$BG+H,LTCYAN$$";
    while (!ScanKey) {
      t0=Beat;
      Play("5eDEqFFetEEFqDeCDDEetCGF");
      Play("5eDEqFFetEEFqDeCDDEetCGF");
      Play("5eDCqDE4eAA5etEEFEDG4B5DCqF");
      Play("5eDCqDE4eAA5etEEFEDG4B5DCqF");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
