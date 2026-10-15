#include "web_server_manager.h"
#include "secrets.h"

#include <FS.h>
#include <SD_MMC.h>
#include <WiFi.h>

namespace {
constexpr char AP_SSID[] = "PillCam";
constexpr char AP_PASSWORD[] = PILL_CAMERA_AP_PASSWORD;
constexpr uint8_t ESPNOW_CHANNEL = 1;
}

WebServerManager::WebServerManager(CameraManager &cameraManager, SdManager &sdManager)
    : _cameraManager(cameraManager), _sdManager(sdManager), _server(80) {}

void WebServerManager::begin() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD, ESPNOW_CHANNEL);

  Serial.print("Camera web AP: ");
  Serial.println(AP_SSID);
  Serial.print("Camera web IP: ");
  Serial.println(WiFi.softAPIP());

  addLog("Web AP started");

  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/preview.jpg", HTTP_GET, [this]() { handlePreviewJpeg(); });
  _server.on("/capture", HTTP_POST, [this]() { handleCapture(); });
  _server.on("/photos", HTTP_GET, [this]() { handlePhotos(); });
  _server.on("/photo", HTTP_GET, [this]() { handlePhotoFile(); });
  _server.on("/logs", HTTP_GET, [this]() { handleLogs(); });
  _server.onNotFound([this]() { _server.send(404, "text/plain", "Not found"); });
  _server.begin();
}

void WebServerManager::update() {
  _server.handleClient();
}

void WebServerManager::addLog(const String &message) {
  _logs[_logIndex] = String(millis() / 1000) + "s: " + message;
  _logIndex = (_logIndex + 1) % 20;
  if (_logCount < 20) {
    ++_logCount;
  }
  Serial.println(message);
}

void WebServerManager::handleRoot() {
  const String html = R"HTML(
<!doctype html>
<html>
<head>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>PillCam</title>
  <style>
    body{font-family:Arial,sans-serif;margin:16px;background:#101418;color:#f3f5f7}
    h1{font-size:28px;margin:0 0 12px}
    img{width:100%;max-width:640px;background:#000;border:1px solid #333}
    button{font-size:20px;padding:12px 16px;margin:12px 0;border:0;background:#46d369;color:#08120b}
    section{margin-top:18px}
    a{color:#8fd3ff}
    pre{white-space:pre-wrap;background:#171d23;padding:10px;min-height:80px}
    li{margin:8px 0}
  </style>
</head>
<body>
  <h1>PillCam</h1>
  <img id="preview" src="/preview.jpg">
  <br>
  <button onclick="capture()">Fa poza</button>
  <section>
    <h2>Poze pe card</h2>
    <div id="photos">Se incarca...</div>
  </section>
  <section>
    <h2>Comunicare / log</h2>
    <pre id="logs"></pre>
  </section>
<script>
function refreshPreview(){document.getElementById('preview').src='/preview.jpg?t='+Date.now();}
function loadPhotos(){fetch('/photos').then(r=>r.text()).then(t=>photos.innerHTML=t);}
function loadLogs(){fetch('/logs').then(r=>r.text()).then(t=>logs.textContent=t);}
function capture(){
  fetch('/capture',{method:'POST'}).then(r=>r.text()).then(t=>{
    loadPhotos(); loadLogs(); refreshPreview(); alert(t);
  });
}
setInterval(refreshPreview, 300);
setInterval(loadLogs, 1500);
setInterval(loadPhotos, 5000);
loadPhotos(); loadLogs();
</script>
</body>
</html>
)HTML";

  _server.send(200, "text/html", html);
}

void WebServerManager::handlePreviewJpeg() {
  camera_fb_t *frameBuffer = _cameraManager.capture();
  if (!frameBuffer) {
    _server.send(500, "text/plain", "Capture failed");
    return;
  }

  _server.sendHeader("Cache-Control", "no-store");
  _server.setContentLength(frameBuffer->len);
  _server.send(200, "image/jpeg", "");
  WiFiClient client = _server.client();
  client.write(frameBuffer->buf, frameBuffer->len);
  _cameraManager.release(frameBuffer);
}

void WebServerManager::handleCapture() {
  String savedPath;
  if (!captureAndSave(savedPath)) {
    _server.send(500, "text/plain", "Capture failed");
    return;
  }

  addLog("Photo saved: " + savedPath);
  _server.send(200, "text/plain", "Saved " + savedPath);
}

void WebServerManager::handlePhotos() {
  _server.send(200, "text/html", _sdManager.buildPhotoListHtml());
}

void WebServerManager::handlePhotoFile() {
  if (!_sdManager.isReady()) {
    _server.send(500, "text/plain", "SD unavailable");
    return;
  }

  String name = _server.arg("name");
  if (name.length() == 0 || name.indexOf("..") >= 0) {
    _server.send(400, "text/plain", "Bad file name");
    return;
  }

  if (!name.startsWith("/")) {
    name = "/" + name;
  }

  File file = SD_MMC.open(name, FILE_READ);
  if (!file) {
    _server.send(404, "text/plain", "Photo not found");
    return;
  }

  _server.streamFile(file, "image/jpeg");
  file.close();
}

void WebServerManager::handleLogs() {
  String text;
  text.reserve(512);
  for (uint8_t count = 0; count < _logCount; ++count) {
    const uint8_t index = (_logIndex + 20 - _logCount + count) % 20;
    text += _logs[index];
    text += '\n';
  }
  _server.send(200, "text/plain", text);
}

bool WebServerManager::captureAndSave(String &savedPath) {
  if (!_sdManager.isReady()) {
    addLog("Capture failed: SD unavailable");
    return false;
  }

  camera_fb_t *frameBuffer = _cameraManager.capture();
  if (!frameBuffer) {
    addLog("Capture failed: camera unavailable");
    return false;
  }

  char path[32];
  snprintf(path, sizeof(path), "/photo_%05lu.jpg", static_cast<unsigned long>(_sdManager.nextPhotoNumber()));
  const bool saved = _sdManager.saveJpeg(frameBuffer, path);
  _cameraManager.release(frameBuffer);

  if (!saved) {
    addLog("Capture failed: write error");
    return false;
  }

  savedPath = path;
  return true;
}
