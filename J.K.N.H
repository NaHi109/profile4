<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <title>北鉄バス 運転シミュレーター (金沢駅〜香林坊)</title>
    <style>
        body { margin: 0; background: #333; font-family: 'Helvetica Neue', Arial, sans-serif; overflow: hidden; color: white; }
        #game-container { position: relative; width: 100vw; height: 100vh; display: flex; flex-direction: column; align-items: center; }
        canvas { background: #555; box-shadow: 0 0 20px rgba(0,0,0,0.5); cursor: none; }
        #ui { position: absolute; top: 20px; left: 20px; background: rgba(0,0,0,0.7); padding: 15px; border-radius: 8px; border-left: 5px solid #e60012; }
        .info { font-size: 18px; margin-bottom: 5px; }
        .dest { font-size: 24px; font-weight: bold; color: #ffcc00; }
        #controls { position: absolute; bottom: 20px; right: 20px; background: rgba(0,0,0,0.7); padding: 10px; font-size: 14px; border-radius: 5px; }
        #message { position: absolute; top: 40%; width: 100%; text-align: center; font-size: 40px; font-weight: bold; color: #fff; text-shadow: 2px 2px 4px #000; pointer-events: none; }
    </style>
</head>
<body>

<div id="game-container">
    <div id="ui">
        <div class="info">次停車駅: <span id="next-stop" class="dest">金沢駅 (始発)</span></div>
        <div class="info">速度: <span id="speed">0</span> km/h</div>
        <div class="info">乗客の満足度: <span id="satisfaction">100</span>%</div>
    </div>

    <canvas id="gameCanvas"></canvas>

    <div id="message"></div>
    
    <div id="controls">
        操作方法:<br>
        ↑ : 加速 / ↓ : ブレーキ・後退<br>
        ← → : ハンドル操作<br>
        バス停の黄色いエリアで停車してください
    </div>
</div>

<script>
const canvas = document.getElementById('gameCanvas');
const ctx = canvas.getContext('2d');
const nextStopEl = document.getElementById('next-stop');
const speedEl = document.getElementById('speed');
const satisfEl = document.getElementById('satisfaction');
const messageEl = document.getElementById('message');

// キャンバスサイズ設定
canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

// ゲーム状態
const stops = [
    { name: "金沢駅東口", y: 200, reached: false },
    { name: "武蔵ヶ辻・近江町市場", y: -1500, reached: false },
    { name: "南町・尾山神社", y: -3000, reached: false },
    { name: "香林坊", y: -4500, reached: false }
];

let bus = {
    x: canvas.width / 2,
    y: canvas.height - 150,
    width: 40,
    height: 80,
    speed: 0,
    angle: 0,
    maxSpeed: 5,
    accel: 0.1,
    friction: 0.05,
    rotation: 0
};

let worldY = 0;
let keys = {};
let satisfaction = 100;
let currentStopIdx = 0;

window.addEventListener('keydown', e => keys[e.code] = true);
window.addEventListener('keyup', e => keys[e.code] = false);

function update() {
    // 加速・減速
    if (keys['ArrowUp']) bus.speed += bus.accel;
    if (keys['ArrowDown']) bus.speed -= bus.accel * 2;
    
    // 摩擦
    if (bus.speed > 0) bus.speed -= bus.friction;
    if (bus.speed < 0) bus.speed += bus.friction;
    if (Math.abs(bus.speed) < 0.05) bus.speed = 0;

    // 速度制限
    if (bus.speed > bus.maxSpeed) bus.speed = bus.maxSpeed;
    if (bus.speed < -2) bus.speed = -2;

    // ハンドル操作
    if (Math.abs(bus.speed) > 0.1) {
        const turnSpeed = bus.speed > 0 ? 0.03 : -0.03;
        if (keys['ArrowLeft']) bus.angle -= turnSpeed;
        if (keys['ArrowRight']) bus.angle += turnSpeed;
    }

    // 移動計算
    bus.x += Math.sin(bus.angle) * bus.speed;
    worldY += Math.cos(bus.angle) * bus.speed;

    // UI更新
    speedEl.innerText = Math.floor(Math.abs(bus.speed) * 10);
    
    // バス停判定
    checkBusStop();
}

function checkBusStop() {
    let nextStop = stops[currentStopIdx];
    let distance = Math.abs((canvas.height - 200 - worldY) - nextStop.y);

    if (distance < 50 && Math.abs(bus.speed) < 0.5) {
        if (!nextStop.reached) {
            nextStop.reached = true;
            showMessage(nextStop.name + " に到着しました");
            if (currentStopIdx < stops.length - 1) {
                currentStopIdx++;
                nextStopEl.innerText = stops[currentStopIdx].name;
            } else {
                showMessage("終点：香林坊。お疲れ様でした！");
            }
        }
    }
}

function showMessage(txt) {
    messageEl.innerText = txt;
    setTimeout(() => { if(messageEl.innerText === txt) messageEl.innerText = ""; }, 3000);
}

function draw() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // 背景（道路）の描画
    ctx.save();
    ctx.translate(0, worldY);

    // 道路
    ctx.fillStyle = "#444";
    ctx.fillRect(canvas.width/2 - 100, -10000, 200, 20000);
    
    // 白線
    ctx.strokeStyle = "#fff";
    ctx.setLineDash([20, 20]);
    ctx.beginPath();
    ctx.moveTo(canvas.width/2, -10000);
    ctx.lineTo(canvas.width/2, 10000);
    ctx.stroke();

    // バス停の描画
    stops.forEach(stop => {
        ctx.fillStyle = stop.reached ? "rgba(0, 255, 0, 0.3)" : "rgba(255, 255, 0, 0.5)";
        ctx.fillRect(canvas.width/2 - 120, stop.y, 240, 60);
        ctx.fillStyle = "white";
        ctx.font = "20px Bold Arial";
        ctx.fillText("🚏 " + stop.name, canvas.width/2 + 130, stop.y + 35);
    });

    ctx.restore();

    // バス本体の描画
    ctx.save();
    ctx.translate(bus.x, bus.y);
    ctx.rotate(bus.angle);

    // 北鉄バスカラーリング (赤とクリーム)
    ctx.fillStyle = "#f5f5dc"; // クリーム色
    ctx.fillRect(-bus.width/2, -bus.height/2, bus.width, bus.height);
    
    ctx.fillStyle = "#e60012"; // 北鉄レッド
    ctx.fillRect(-bus.width/2, 0, bus.width, bus.height/4); // サイドライン
    ctx.fillRect(-bus.width/2, -bus.height/2, bus.width, 10); // 前面
    
    // 窓
    ctx.fillStyle = "#333";
    ctx.fillRect(-bus.width/2 + 5, -bus.height/2 + 5, bus.width - 10, 15); // フロントガラス
    
    // ライト
    ctx.fillStyle = "yellow";
    ctx.fillRect(-bus.width/2 + 5, -bus.height/2, 8, 5);
    ctx.fillRect(bus.width/2 - 13, -bus.height/2, 8, 5);

    ctx.restore();

    update();
    requestAnimationFrame(draw);
}

draw();
</script>
</body>
</html>
