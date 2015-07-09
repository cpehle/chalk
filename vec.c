#include "u.h"
#include "dat.h"
#include "console.h"
#include "vec.h"

#define I(mat,i,j) (mat.m[i*4+j])


V4 v4z() { return ((V4){0, 0, 0, 0}); }

M44 m44i() { return ((M44){1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1}); }

V4 v4msv(float a, V4 v) {
  V4 w;
  for (int i = 0; i < 4; i++) {
    w.v[i] = a * v.v[i];
  }
  return w;
}

V4 v4avv(V4 v, V4 w) {
  V4 u;
  for (int i = 0; i < 4; i++) {
    u.v[i] = w.v[i] + v.v[i];
  }
  return u;
}

V4 v4mmv(M44 m, V4 v) {
  V4 u = {};
  for (int i = 0; i<4; ++i) {
          for (int j = 0; j<4; ++j) {
                  u.v[i] += I(m,i,j) * v.v[j];
          }
  }
  return u;
}

M44 v4msm(float s, M44 m) {
        M44 o;
        for (int i = 0; i < 16; i++) {
                o.m[i] = s*m.m[i];
        }
        return o;
}


M44 v4amm(M44 m, M44 n) {
        M44 o;
        for (int i = 0; i < 16; i++) {
                o.m[i] = n.m[i] + m.m[i];
        }
        return o;
}

M44 v4mmm(M44 m, M44 n) {
  M44 o = {};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 4; k++) {
              I(o,i,j) += I(m,i,k) * I(n,k,j);
      }
    }
  }
  return o;
}

V4 v4(float x, float y, float z, float t) { return (V4){x, y, z, t}; }

void cprintv4(Console c, V4 v) {
  for (int i = 0; i < 4; i++) {
    cprintint(c, (u32)v.v[i], 16, 0),
        ((i == 3) ? cputc(c, '\n') : cputc(c, ' '));
  }
}

void cprintm44(Console c, M44 m) {
  for (int i = 0; i < 16; i++) {
          cprintint(c, (u32)m.m[i], 16, 0), (((i+1)%4) ? cputc(c, ' ') : cputc(c,'\n'));
  }
}
