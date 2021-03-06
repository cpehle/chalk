typedef union M44 {
  struct {
    float xx, xy, xz, xt,
          yx, yy, yz, yt,
          zx, zy, zz, zt,
          tx, ty, tz, tt;
  };
  struct {
    float mx[4];
    float my[4];
    float mz[4];
    float mt[4];
  };
  float m[4 * 4];
} M44;

typedef union V4 {
  struct {
    float x, y, z, t;
  };
  float v[4];
} V4;



typedef struct Bezier Bezier;
typedef struct BezierData BezierData;
typedef V4 (*BezierEv)(BezierData b, float t);
struct BezierData {
        int nc;
        V4 cp[4];
};
struct Bezier {
  BezierEv bezierev;
  BezierData bezierdata;
};

V4 v4(float, float, float, float);
V4 v4msv(float a, V4 v);
V4 v4avv(V4 v, V4 w);
V4 v4mmv(M44 m, V4 v);
M44 v4msm(float a, M44 v);
M44 v4amm(M44 v, M44 w);
M44 v4mmm(M44 m, M44 v);
M44 m44i();
V4  v4z();

V4 evbezier(Bezier b, float t);
V4 evcubicbezier(BezierData b, float t);
Bezier cubicbezier(BezierData b);

V4 evbezier(Bezier b, float t);

void cprintv4(Console c, V4 v);
void cprintm44(Console c, M44 m);
