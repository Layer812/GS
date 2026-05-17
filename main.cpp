// GRAVITY SPIN / Layer812
// 

#include <Arduino.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <math.h>

// ============================================================
// M5GFX (AtomS3R用 直接駆動)
// ============================================================
M5GFX& display = M5.Display;
LGFX_Sprite canvas(&display);
LGFX_Sprite mazeSprite(&canvas);

// ============================================================
// 定数定義
// ============================================================
#define SCREEN_W        128
#define SCREEN_H        128
#define MAZE_SIZE       512
#define WALL_COLOR      0xFFFF     // TFT_WHITE
#define BG_COLOR        0x0000     // TFT_BLACK
#define BALL_COLOR      0xF800     // TFT_RED

#define BALL_RADIUS     21.0f      // 画面上での半径（直径42）
#define BALL_R_MAZE     6.0f       // 迷路内での判定半径

#define GRAVITY_CONST   3.0f
#define FRICTION        0.97f
#define ELAPSED_TIME    1.0f
#define MAX_SPEED       10.0f
#define ANGLE_SMOOTH    64
#define ANGLE_DEADZONE  5.0f      // 傾き検出のデッドゾーン（度）
#define TILT_THRESHOLD  10.0f     // 重力を有効にする最小傾斜角（度）
#define SMOOTH_FACTOR   0.05f     // 1次遅れフィルタ係数

// ============================================================
// グローバル変数
// ============================================================
struct Ball {
    float x, y;
    float vx, vy;
    float r;
};
Ball ball;

float mazeAngle = 0.0f;
float zoomLevel = 2.0f;
float angleHistory[ANGLE_SMOOTH];
int angleIdx = 0;
bool imuReady = false;

// ============================================================
// ゲーム状態管理
// ============================================================
unsigned long playStartTime = 0;
float finalTime = 0.0f;
bool stageCleared = false; // 現在のステージをクリアしたフラグ
bool isTimeUp = false;      // 99秒タイムアップによるFAILフラグ
int currentStage = 0;       // 現在のステージ (0 = Stage 1, 4 = Stage 5)
bool isTitle = true;        // 起動時タイトル画面フラグ
float totalTime = 0.0f;     // 全5ステージの累計タイムタイム

// ステージごとのゴール座標 (セル座標)
static const int goalX[5] = {14, 1, 14, 8, 14}; // 4ステージ目のゴールは (8, 8)
static const int goalY[5] = {14, 14, 3, 8, 14};

// ステージごとのスタート座標 (セル座標)
static const int startX[5] = {8, 8, 8, 1, 8};   // 4ステージ目は左上端っこ (1, 1)
static const int startY[5] = {8, 8, 8, 1, 8};

// ============================================================
// 迷路データ 16x16 (全5面)
// ============================================================
static const uint8_t mazeData[5][16][16] = {
  // STAGE 1 (初級)
  {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,1,1,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,0,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
  },
  // STAGE 2 (渦巻き状・中級)
  {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
  },
  // STAGE 3 (トリッキー・ジグザグ)
  {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,1,0,1,0,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},
    {1,1,1,0,1,1,1,1,0,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1},
    {1,0,1,1,1,1,0,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,0,1,0,1,0,0,0,1,0,0,0,1},
    {1,0,1,0,0,1,0,1,1,1,1,1,1,1,0,1},
    {1,0,1,1,0,1,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
  },
  // STAGE 4 (周囲の壁のみ、中央にゴール)
  {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
  },
  // STAGE 5 (ラスト・難関)
  {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,1,0,0,1,0,0,0,1,0,1},
    {1,1,1,1,1,0,1,0,0,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,1,0,0,1,0,1,0,0,0,1},
    {1,0,1,0,1,0,1,1,1,1,0,1,0,1,1,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,1,0,0,1,0,0,0,1,0,1},
    {1,0,1,1,1,0,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
    {1,1,1,0,1,0,1,1,1,1,0,1,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
  }
};

// 関数宣言
void drawMazeToSprite(void);
void updateImuAngle(void);
void updatePhysics(void);
bool updateCollision(Ball &b);
void renderFrame(void);

// 衝突判定 (16点サンプリング)
bool updateCollision(Ball &b) {
  const int pts = 16;
  bool collided = false;
  float pushX = 0.0f, pushY = 0.0f;
  float maxOverlap = 0.0f;

  for (int i = 0; i < pts; i++) {
    float ang = (float)i * (2.0f * PI) / (float)pts;
    float cosA = cosf(ang);
    float sinA = sinf(ang);

    float checkX = b.x + cosA * b.r;
    float checkY = b.y + sinA * b.r;

    int ix = (int)checkX;
    int iy = (int)checkY;

    bool isWall = false;
    if (ix < 0 || ix >= MAZE_SIZE || iy < 0 || iy >= MAZE_SIZE) {
      isWall = true;
    } else {
      uint16_t color = mazeSprite.readPixel(ix, iy);
      if (color == WALL_COLOR) isWall = true;
    }

    if (isWall) {
      float distFromCenter = sqrtf((checkX - b.x) * (checkX - b.x) + (checkY - b.y) * (checkY - b.y));
      float overlap = b.r - (distFromCenter - b.r);
      if (overlap > 0.0f) {
        pushX -= cosA * overlap;
        pushY -= sinA * overlap;
        if (overlap > maxOverlap) maxOverlap = overlap;
        collided = true;
      }
    }
  }

  if (collided) {
    float pushMag = sqrtf(pushX * pushX + pushY * pushY);
    if (pushMag > 0.001f) {
      pushX /= pushMag;
      pushY /= pushMag;

      b.x += pushX * maxOverlap;
      b.y += pushY * maxOverlap;

      float dot = b.vx * pushX + b.vy * pushY;
      if (dot < 0) {
        b.vx -= 2.0f * dot * pushX;
        b.vy -= 2.0f * dot * pushY;
        b.vx *= 0.8f;
        b.vy *= 0.8f;
      }
    }
  }
  return collided;
}

void drawMazeToSprite(void) {
  int cellSize = MAZE_SIZE / 16;
  mazeSprite.fillSprite(BG_COLOR);
  int gx = goalX[currentStage];
  int gy = goalY[currentStage];

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      if (mazeData[currentStage][y][x] == 1) {
        mazeSprite.fillRect(x * cellSize, y * cellSize, cellSize, cellSize, WALL_COLOR);
      } else if (x == gx && y == gy) {
        // 現在のステージのゴールセルを赤く塗る
        mazeSprite.fillRect(x * cellSize, y * cellSize, cellSize, cellSize, 0xF800); // TFT_RED
        // "G"マークを白で描画
        mazeSprite.setTextColor(0xFFFF);
        mazeSprite.setTextDatum(MC_DATUM);
        mazeSprite.drawString("G", x * cellSize + cellSize / 2, y * cellSize + cellSize / 2, &fonts::Font2);
      }
    }
  }
}

// IMU角度更新 (加速度ローパスフィルタによる360度境界バグ完全対応)
void updateImuAngle(void) {
  if (!imuReady) return;

  float ax = 0.0f, ay = 0.0f, az = 0.0f;
  M5.Imu.update();
  M5.Imu.getAccel(&ax, &ay, &az);
  
  // 静的変数でフィルタリング後の加速度を保持 (初期状態は正立)
  static float filtered_ax = 0.0f;
  static float filtered_ay = 1.0f;

  // 1次遅れフィルタを加速度ベクトル自体に適用 (手ブレ・振動の除去)
  // これにより角度の不連続点(-180度 / 180度)での平均化バグが100%根本解消されます
  filtered_ax = filtered_ax * (1.0f - SMOOTH_FACTOR) + ax * SMOOTH_FACTOR;
  filtered_ay = filtered_ay * (1.0f - SMOOTH_FACTOR) + ay * SMOOTH_FACTOR;

  // フィルタリングされたベクトルから角度を計算 (X軸の符号反転により、90度/270度の反転問題を解消)
  float targetAngle = atan2f(-filtered_ax, filtered_ay) * 180.0f / PI;

  float absTilt = fabsf(targetAngle);
  if (absTilt < ANGLE_DEADZONE) {
    mazeAngle = mazeAngle * 0.8f;
    if (fabsf(mazeAngle) < 0.1f) mazeAngle = 0.0f;
    return;
  }

  // ターゲット角度への滑らかな遷移
  float delta = targetAngle - mazeAngle;
  while (delta > 180.0f)  delta -= 360.0f;
  while (delta < -180.0f) delta += 360.0f;

  mazeAngle += delta * SMOOTH_FACTOR;
}

// 物理演算 (キャメルトライ重力回転モデル)
void updatePhysics(void) {
  if (stageCleared || isTimeUp) return; // クリアまたはタイムアップ時は停止

  // タイムアップ判定
  float elapsed = (float)(millis() - playStartTime) / 1000.0f;
  if (elapsed >= 99.0f) {
    isTimeUp = true;
    return;
  }

  // ゴール判定
  int cellX = (int)ball.x / 32;
  int cellY = (int)ball.y / 32;
  if (cellX == goalX[currentStage] && cellY == goalY[currentStage]) {
    stageCleared = true;
    finalTime = elapsed;
    totalTime += finalTime; // 累計クリアタイムに加算！
    return;
  }

  // スプライトの回転とピボットによるスクロール逆変換の相互作用を解決するため、角度を2倍にします
  float rad = (mazeAngle * 2.0f) * PI / 180.0f;
  float gx = GRAVITY_CONST * sinf(rad); 
  float gy = GRAVITY_CONST * cosf(rad); 

  ball.vx += gx * ELAPSED_TIME;
  ball.vy += gy * ELAPSED_TIME;
  ball.vx *= FRICTION;
  ball.vy *= FRICTION;

  float speed = sqrtf(ball.vx * ball.vx + ball.vy * ball.vy);
  if (speed > MAX_SPEED) {
    ball.vx = (ball.vx / speed) * MAX_SPEED;
    ball.vy = (ball.vy / speed) * MAX_SPEED;
  }

  ball.x += ball.vx * ELAPSED_TIME;
  ball.y += ball.vy * ELAPSED_TIME;

  updateCollision(ball);

  ball.x = constrain(ball.x, ball.r, (float)MAZE_SIZE - ball.r);
  ball.y = constrain(ball.y, ball.r, (float)MAZE_SIZE - ball.r);
}

// 描画 (M5GFX直接駆動)
void renderFrame(void) {
  canvas.fillScreen(0x0000);

  if (isTitle) {
    // ============================================================
    // タイトル画面の描画
    // ============================================================
    // 迷路の中心 (256, 256) をピボットにして、ゆっくり自動回転
    mazeSprite.setPivot(256, 256);
    canvas.setPivot(64, 64);
    mazeSprite.pushRotateZoom(&canvas, mazeAngle, zoomLevel, zoomLevel);

    // 画面全体にスキャンライン風の半透明黒を重ねる (ロゴを際立たせる)
    for (int y = 0; y < SCREEN_H; y += 2) {
      canvas.drawFastHLine(0, y, SCREEN_W, 0x0000);
    }
    
    // ボールと同じネオンゴールドのグラデーション立体プレート背景
    canvas.fillRect(4, 20, SCREEN_W - 8, 48, 0x1800); // 1. 高級感あるディープゴールド影
    canvas.fillRect(4, 20, SCREEN_W - 8, 46, 0xFDE0); // 2. ネオンゴールド・ベース
    canvas.fillRect(6, 22, SCREEN_W - 12, 42, 0xFEE0); // 3. グラデーション
    canvas.fillRect(8, 24, SCREEN_W - 16, 38, 0xFFE0); // 4. ハイライトプレート
    canvas.drawRect(4, 20, SCREEN_W - 8, 46, 0xFFFF); // 5. ホワイトの極細外枠
    
    // 独創的なSFタイトル「GRAVITY SPIN」を影付き3D立体文字に！
    canvas.setTextDatum(MC_DATUM);
    
    // 影文字（ブラウン 0x4800）を右下に1ドットずらして描画
    canvas.setTextColor(0x4800);
    canvas.drawString("GRAVITY", SCREEN_W / 2 + 1, 33 + 1, &fonts::Font2);
    canvas.drawString("SPIN", SCREEN_W / 2 + 1, 53 + 1, &fonts::Font2);
    
    // 主文字（ネオンレッド 0xF800）を重ねて描画して飛び出る立体感！
    canvas.setTextColor(0xF800);
    canvas.drawString("GRAVITY", SCREEN_W / 2, 33, &fonts::Font2);
    canvas.drawString("SPIN", SCREEN_W / 2, 53, &fonts::Font2);
    
    // サブタイトル
    canvas.setTextColor(0x0000); // ゴールド背景の上で見やすい黒
    canvas.drawString("THE TILT MAZE", SCREEN_W / 2, 80, &fonts::Font0);

    // 点滅する「Push to Start」
    if ((millis() / 500) % 2 == 0) {
      canvas.setTextColor(0xF800); // 鮮烈な赤
      canvas.drawString("Push to Start", SCREEN_W / 2, 106, &fonts::Font2);
    }
    
  } else {
    // ============================================================
    // ゲームプレイ中の描画
    // ============================================================
    mazeSprite.setPivot((int)ball.x, (int)ball.y);
    canvas.setPivot(64, 64);
    
    // 迷路の回転描画
    mazeSprite.pushRotateZoom(&canvas, mazeAngle, zoomLevel, zoomLevel);

    // 球（ボール）の立体的な描画 (数字重ねは廃止し3D質感を極限に向上)
    int cx = 64;
    int cy = 64;
    int r = (int)BALL_RADIUS;
    
    // 1. シャドウ (暗いレッドシアンで高級感ある立体影)
    canvas.fillCircle(cx + 2, cy + 2, r, 0x1800);
    // 2. 球体ベース (美しいネオンイエロー・ゴールド系)
    canvas.fillCircle(cx, cy, r, 0xFDE0); 
    // 3. グラデーション効果
    canvas.fillCircle(cx - 3, cy - 3, r - 5, 0xFEE0);
    canvas.fillCircle(cx - 5, cy - 5, r - 10, 0xFFE0);
    // 4. ハイライト (ホワイトのツヤ表現)
    canvas.fillCircle(cx - 8, cy - 8, 3, 0xFFFF);

    // 制限時間（99秒からのカウントダウン）の計算
    char timeStr[16];
    float timeLeft = 99.0f;
    if (stageCleared) {
      timeLeft = 99.0f - finalTime;
    } else if (isTimeUp) {
      timeLeft = 0.0f;
    } else {
      float elapsed = (float)(millis() - playStartTime) / 1000.0f;
      timeLeft = 99.0f - elapsed;
      if (timeLeft <= 0.0f) {
        timeLeft = 0.0f;
        isTimeUp = true;
      }
    }
    snprintf(timeStr, sizeof(timeStr), "%.1f", timeLeft);

    // 【HUD】右上のタイムカウントダウン表示 (青/赤の背景の上に白文字)
    uint16_t bgColor = 0x015F;     // 通常ロイヤルブルー
    uint16_t borderColor = 0x03FF; // 通常ネオンブルー
    uint16_t textColor = 0xFFFF;   // 白文字

    if (timeLeft <= 10.0f && !stageCleared) {
      bgColor = 0x8000;     // 警告ダークレッド
      borderColor = 0xF800; // ネオンレッド
      textColor = 0xFFE0;   // 黄色文字（警告色）
    }

    canvas.fillRoundRect(82, 4, 42, 16, 4, bgColor);
    canvas.drawRoundRect(82, 4, 42, 16, 4, borderColor);
    
    canvas.setTextColor(textColor);
    canvas.setTextDatum(MC_DATUM);
    canvas.drawString(timeStr, 103, 12, &fonts::Font0);

    // 【HUD】左上のステージ表示
    canvas.setTextColor(0xFFFF);
    canvas.setTextDatum(TL_DATUM);
    canvas.setCursor(2, 2);
    canvas.printf("STAGE %d/5", currentStage + 1);

    // ============================================================
    // 【演出】ゴールおよびFAIL画面表示
    // ============================================================
    if (isTimeUp) {
      // タイムアップ FAIL 画面 (フォント2に縮小して画面内に完璧に収めます)
      for (int y = 20; y < 110; y += 2) canvas.drawFastHLine(0, y, SCREEN_W, 0x0000);
      canvas.fillRect(0, 32, SCREEN_W, 64, 0x8000); // 警告赤背景
      canvas.drawRect(0, 32, SCREEN_W, 64, 0xF800); // 枠線
      
      canvas.setTextColor(0xFFFF); // 白
      canvas.setTextDatum(MC_DATUM);
      canvas.drawString("STAGE FAIL !", SCREEN_W / 2, 50, &fonts::Font2);
      canvas.drawString("TIME UP (99s)", SCREEN_W / 2, 74, &fonts::Font2);
      
      canvas.setTextColor(0xFFE0); // 黄色
      canvas.drawString("Tap Screen to Retry", SCREEN_W / 2, 110, &fonts::Font0);
      
    } else if (stageCleared) {
      if (currentStage < 4) {
        // 通常のステージクリア画面
        for (int y = 20; y < 110; y += 2) canvas.drawFastHLine(0, y, SCREEN_W, 0x0000);
        canvas.fillRect(0, 32, SCREEN_W, 64, 0x015F); // ディープブルー背景
        canvas.drawRect(0, 32, SCREEN_W, 64, 0x03FF); // ネオンブルー枠
        
        canvas.setTextColor(0x07FF); // 輝くシアン
        canvas.setTextDatum(MC_DATUM);
        canvas.drawString("STAGE CLEAR !", SCREEN_W / 2, 50, &fonts::Font2);
        
        char finalStr[32];
        snprintf(finalStr, sizeof(finalStr), "STAGE %d: %.2fs", currentStage + 1, finalTime);
        canvas.setTextColor(0xFFFF);
        canvas.drawString(finalStr, SCREEN_W / 2, 74, &fonts::Font2);
        
        canvas.setTextColor(0xFFE0); // 黄色
        canvas.drawString("Tap to Next Stage", SCREEN_W / 2, 110, &fonts::Font0);
      } else {
        // 5面完全制覇 (ALL CLEAR) 画面
        for (int y = 20; y < 110; y += 2) canvas.drawFastHLine(0, y, SCREEN_W, 0x0000);
        canvas.fillRect(0, 32, SCREEN_W, 64, 0xE7E0); // 高級感のあるゴールドプレート
        canvas.drawRect(0, 32, SCREEN_W, 64, 0xFFFF); // 白枠
        
        canvas.setTextColor(0x0000); // 黒文字でクッキリ
        canvas.setTextDatum(MC_DATUM);
        canvas.drawString("ALL CLEAR !!", SCREEN_W / 2, 50, &fonts::Font2);
        
        char finalStr[32];
        snprintf(finalStr, sizeof(finalStr), "TOTAL TIME: %.2fs", totalTime);
        canvas.drawString(finalStr, SCREEN_W / 2, 74, &fonts::Font2);
        
        canvas.setTextColor(0xFFFF);
        canvas.drawString("Tap to Return Title", SCREEN_W / 2, 110, &fonts::Font0);
      }
    }
  }

  canvas.pushSprite(&M5.Display, 0, 0);
}

void setup() {
  // M5Unifiedの初期化 (ディスプレイも自動で完璧に初期化されます)
  auto cfg = M5.config();
  M5.begin(cfg);

  // 液晶画面の設定
  M5.Display.setRotation(0);
  M5.Display.fillScreen(0x0000);

  canvas.setColorDepth(16);
  if (!canvas.createSprite(SCREEN_W, SCREEN_H)) {
    while (1) delay(1);
  }

  mazeSprite.setColorDepth(16);
  mazeSprite.setPsram(true);
  if (!mazeSprite.createSprite(MAZE_SIZE, MAZE_SIZE)) {
    while (1) delay(1);
  }

  drawMazeToSprite();
  mazeSprite.setBaseColor(BG_COLOR);

  // M5UnifiedによりIMU(BMI270)を安全・最適に初期化
  if (M5.Imu.begin()) {
    imuReady = true;
    Serial.println("IMU initialized successfully via M5Unified!");
  } else {
    Serial.println("IMU initialization FAILED!");
  }

  for (int i = 0; i < ANGLE_SMOOTH; i++) angleHistory[i] = 0.0f;

  ball.x = startX[currentStage] * 32.0f + 16.0f;
  ball.y = startY[currentStage] * 32.0f + 16.0f;
  ball.vx = 0.0f;
  ball.vy = 0.0f;
  ball.r = BALL_R_MAZE;

  playStartTime = millis(); // ゲームの計測時間をスタート
}

void loop() {
  M5.update(); // ボタンや画面タップの判定を更新

  if (isTitle) {
    // タイトル画面で画面が押し込まれたらゲームスタート！
    if (M5.BtnA.wasPressed()) {
      isTitle = false;
      stageCleared = false;
      isTimeUp = false;
      currentStage = 0;
      totalTime = 0.0f; // トータルタイムを初期化！
      drawMazeToSprite();
      ball.x = startX[currentStage] * 32.0f + 16.0f;
      ball.y = startY[currentStage] * 32.0f + 16.0f;
      ball.vx = 0.0f;
      ball.vy = 0.0f;
      playStartTime = millis();
    }
    // 背景の迷路をゆっくり自動回転 (タイトル画面の演出)
    mazeAngle += 0.4f;
    if (mazeAngle >= 360.0f) mazeAngle -= 360.0f;

  } else {
    // ゲームプレイ中、またはクリア・FAIL時のボタン制御
    if (M5.BtnA.wasPressed()) {
      if (isTimeUp) {
        // タイムアップFAIL時：画面タップで即座に「タイトル画面」に戻る！
        isTimeUp = false;
        isTitle = true;
        currentStage = 0;
        drawMazeToSprite();
      } else if (stageCleared) {
        if (currentStage < 4) {
          // ステージクリア時：次のステージへ進行
          stageCleared = false;
          currentStage++;
          drawMazeToSprite(); // 新しい迷路を描き直す！
          ball.x = startX[currentStage] * 32.0f + 16.0f;
          ball.y = startY[currentStage] * 32.0f + 16.0f;
          ball.vx = 0.0f;
          ball.vy = 0.0f;
          playStartTime = millis();
        } else {
          // 5面完全クリア時：タイトル画面に戻る！
          stageCleared = false;
          isTitle = true;
          currentStage = 0;
          drawMazeToSprite();
        }
      } else {
        // プレイ中にボタンが押された場合：即座に中断してタイトル画面に戻る！！！
        isTitle = true;
        currentStage = 0;
        drawMazeToSprite();
      }
    }

    updateImuAngle();
    updatePhysics();
  }

  renderFrame();
  delay(16);
}