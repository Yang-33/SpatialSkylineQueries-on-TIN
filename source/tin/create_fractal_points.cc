#include <algorithm>
#include <cmath>
#include <iostream>

const int POINT_NUM_SQRT = 142;  // ! You can modify this parameter.
const int HEIGHT_MAX = 266;
const int rseed = 0;  // default : 0 ! You can modify this parameter.
// POINT_NUM_SQRT | HEIGHT_MAX(height)
// 24*24 = 576 | 50
// 32*32 = 1024 | 74
// 45*45 = 2025 | 77
// 55*55 = 3025 | 110
// 64*64 = 4096 | 140
// 71*71 = 5041 | 170
// 78*78 = 6084 | 175
// 84*84 = 7056 | 179
// 90*90 = 8100 | 190
// 95*95 = 9025 | 205
// 100*100 = 10000 | 210
// 105*105 = 11025 | 217
// 110*110 = 12100 | 221
// 115*115 = 13225 | 225
// 119*119 = 14161 | 234
// 123*123 = 15129 | 240
// 127*127 = 16129 | 245
// 131*131 = 17161 | 251
// 135*135 = 18225 | 255
// 138*138 = 19044 | 260
// 142*142 = 20164 | 266
// 224*224 = 50176 | 260, 330
// 317*317 = 100489 | NG
// 333*333 = 110889 | 400 (remove z=0, then the size will be 99185)

// settings
static const int SMOOTH_COEF = 4;  // default : 4
const int HASH_CODE_MAX = 256;     // default : 256

// ref:
// https://qiita.com/keny30827/items/f4e29a4a90779cf94da6#%E4%B8%AD%E7%82%B9%E5%A4%89%E4%BD%8D%E6%B3%95

#define CLIP(e, l, h) (min(max(e, l), h))
#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))
using namespace std;

#define VECTOR SVector2D<float>
template <class T>
struct SVector2D {
  typedef T DataType;
  T x;
  T y;
  SVector2D() { Init(); }
  void Init() {
    x = T();
    y = T();
  }
  SVector2D operator+(const SVector2D& e) const {
    SVector2D tmp;
    tmp.x = x + e.x;
    tmp.y = y + e.y;
    return tmp;
  }
  SVector2D& operator+=(const SVector2D& e) {
    x += e.x;
    y += e.y;
    return (*this);
  }
  SVector2D operator-(const SVector2D& e) const {
    SVector2D tmp;
    tmp.x = x - e.x;
    tmp.y = y - e.y;
    return tmp;
  }
  SVector2D& operator-=(const SVector2D& e) {
    x -= e.x;
    y -= e.y;
    return (*this);
  }
  T operator*(const SVector2D& e) const { return (x * e.x) + (y * e.y); }
  SVector2D& operator*=(const int e) {
    x *= e;
    y *= e;
    return (*this);
  }
  SVector2D& operator*=(const float e) {
    x *= e;
    y *= e;
    return (*this);
  }
  SVector2D& operator/=(const int e) {
    x /= e;
    y /= e;
    return (*this);
  }
  SVector2D& operator/=(const float e) {
    x /= e;
    y /= e;
    return (*this);
  }
};

int g_NoiseValue[POINT_NUM_SQRT][POINT_NUM_SQRT];

void SetupMidpointDisplaceNoise(VECTOR topLeft,
                                VECTOR rightBottom,
                                int heightMax) {
  int nTop = (int)floorf(topLeft.y);
  int nLeft = (int)floorf(topLeft.x);
  int nBottom = (int)floorf(rightBottom.y);
  int nRight = (int)floorf(rightBottom.x);

  const int nTopLeft = g_NoiseValue[nLeft][nTop];
  const int nTopRight = g_NoiseValue[nRight][nTop];
  const int nBottomLeft = g_NoiseValue[nLeft][nBottom];
  const int nBottomRight = g_NoiseValue[nRight][nBottom];

  int nX = (nLeft + nRight) / 2;
  int nY = (nTop + nBottom) / 2;

  if (heightMax <= 1) {
    int value =
        (nTopLeft + nTopRight + nBottomLeft + nBottomRight) / SMOOTH_COEF;
    value = CLIP(value, 0, HASH_CODE_MAX - 1);
    if (g_NoiseValue[nX][nY] < 0) {
      g_NoiseValue[nX][nY] = value;
    }
  } else {
    int value =
        (nTopLeft + nTopRight + nBottomLeft + nBottomRight) / SMOOTH_COEF;
    value += (rand() % heightMax) - (heightMax / 2);
    value = CLIP(value, 0, HASH_CODE_MAX - 1);
    if (g_NoiseValue[nX][nY] < 0) {
      g_NoiseValue[nX][nY] = value;
    }

    {
      if (g_NoiseValue[nX][nTop] < 0) {
        g_NoiseValue[nX][nTop] = (nTopLeft + nTopRight) / 2;
      }
      if (g_NoiseValue[nX][nBottom] < 0) {
        g_NoiseValue[nX][nBottom] = (nBottomLeft + nBottomRight) / 2;
      }
      if (g_NoiseValue[nLeft][nY] < 0) {
        g_NoiseValue[nLeft][nY] = (nTopLeft + nBottomLeft) / 2;
      }
      if (g_NoiseValue[nRight][nY] < 0) {
        g_NoiseValue[nRight][nY] = (nTopRight + nBottomRight) / 2;
      }
    }

    {
      VECTOR midPoint;
      midPoint.x = (float)nX;
      midPoint.y = (float)nY;
      VECTOR midUpEdge;
      midUpEdge.x = (float)nX;
      midUpEdge.y = (float)nTop;
      VECTOR midDownEdge;
      midDownEdge.x = (float)nX;
      midDownEdge.y = (float)nBottom;
      VECTOR midLeftEdge;
      midLeftEdge.x = (float)nLeft;
      midLeftEdge.y = (float)nY;
      VECTOR midRightEdge;
      midRightEdge.x = (float)nRight;
      midRightEdge.y = (float)nY;

      heightMax /= 2;
      SetupMidpointDisplaceNoise(topLeft, midPoint, heightMax);
      SetupMidpointDisplaceNoise(midUpEdge, midRightEdge, heightMax);
      SetupMidpointDisplaceNoise(midLeftEdge, midDownEdge, heightMax);
      SetupMidpointDisplaceNoise(midPoint, rightBottom, heightMax);
    }
  }
}

void SetupMidpointDisplaceNoiseINIT(unsigned int seed) {
  srand(seed);

  for (int i = 0; i < POINT_NUM_SQRT; ++i) {
    for (int j = 0; j < POINT_NUM_SQRT; ++j) {
      g_NoiseValue[i][j] = -1;
    }
  }

  g_NoiseValue[0][0] = (rand() % HASH_CODE_MAX);
  g_NoiseValue[POINT_NUM_SQRT - 1][0] = (rand() % HASH_CODE_MAX);
  g_NoiseValue[0][POINT_NUM_SQRT - 1] = (rand() % HASH_CODE_MAX);
  g_NoiseValue[POINT_NUM_SQRT - 1][POINT_NUM_SQRT - 1] =
      (rand() % HASH_CODE_MAX);

  VECTOR topLeft;
  topLeft.x = topLeft.y = 0.0f;
  VECTOR rightBottom;
  rightBottom.x = rightBottom.y = (float)(POINT_NUM_SQRT - 1);
  SetupMidpointDisplaceNoise(topLeft, rightBottom, HASH_CODE_MAX);
}

float GetMidpointDisplaceNoise(float x, float y) {
  int xi = (int)floorf(x);
  int yi = (int)floorf(y);
  return ((float)g_NoiseValue[xi][yi] / (float)(HASH_CODE_MAX - 1));
}

int main() {
  SetupMidpointDisplaceNoiseINIT(rseed);
  cout << "OFF" << endl;
  cout << POINT_NUM_SQRT * POINT_NUM_SQRT << " 0 0 " << endl;
  cout << endl;
  for (int i = 0; i < POINT_NUM_SQRT; ++i) {
    for (int j = 0; j < POINT_NUM_SQRT; ++j) {
      float x = (float)i;
      float y = (float)j;
      const int r = (int)((float)HEIGHT_MAX * GetMidpointDisplaceNoise(x, y));
      std::cout << i << " " << j << " " << r / 2 << std::endl;
    }
  }
}
