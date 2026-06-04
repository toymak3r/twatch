# T-Watch S3 — Relógio "MAINFRAME" com deep sleep (design)

**Data:** 2026-06-04
**Hardware confirmado:** LilyGo T-Watch S3 — ESP32-S3 (rev v0.2), 8MB PSRAM, 16MB flash, PMU **AXP2101**, RTC **PCF8563**, acelerômetro **BMA423**, touch **FT6X36**, display **ST7789 240×240** (LCD com backlight em GPIO45).
**Porta de gravação:** COM5 (USB-Serial/JTAG nativo do S3).

## 1. Problema e objetivo

A bateria dura pouco porque o firmware atual mantém a tela ligada e a CPU sem dormir. Como o display é **LCD** (não OLED), fundo escuro não economiza — só importam **nível de brilho** e **tempo de tela ligada**. Meta: **≥ 20h** de autonomia (esperado: vários dias) sem abrir mão de um relógio utilizável.

## 2. Decisões aprovadas (requisitos)

| Tópico | Decisão |
|---|---|
| Estilo visual | **MAINFRAME** — terminal CRT verde-fósforo, hora vermelha central, segundos laranja discretos |
| Modo ocioso | **Deep sleep** (CPU + tela 100% off) |
| Despertar | **Somente o botão físico** (PWRKEY do AXP2101 → IRQ no GPIO21) |
| Timeout de tela | **10 s** sem interação; qualquer toque/gesto reinicia |
| WiFi | **Só NTP**: 1× no boot frio + 1×/dia; desligado o resto do tempo |
| Gestos | **Leves**: swipe ↑/↓ troca página; double-tap força sync NTP |
| Pedômetro | **Ligado, mínimo**: BMA423 em step-counter de hardware, mantido vivo no deep sleep |
| Brilho ligado | 80/255 (configurável) |

## 3. Stack (decisão de arquitetura)

Sketch **autocontido e enxuto**, **sem LVGL e sem o wrapper `LilyGoLib`** (que hoje está inconsistente: herda de `TFT_eSPI` mas o `platformio.ini` lista LovyanGFX — o repo não compila como está).

- **LovyanGFX** — desenho direto de texto/linhas (face MAINFRAME). Boot rápido = wake rápido.
- **XPowersLib (AXP2101)** — energia, rails, botão. Reaproveita a **sequência de init de rails provada** em `src/LilyGoLib.cpp::beginPower()`.
- **SensorLib** — PCF8563 (hora) + BMA423 (passos).
- **WiFi + `configTime`/SNTP** nativos do ESP32 — sync de hora.
- **`esp_deep_sleep`** com wake `ext1` no pino do botão (GPIO21, nível baixo).
- Pinos vêm de `src/utilities.h` (bloco `USING_TWATCH_S3`), incluído sem o wrapper.

`platformio.ini` será reescrito para um conjunto consistente: `lib_deps = LovyanGFX, XPowersLib, SensorLib`; remover `lvgl`, `arduinoFFT` e todas as flags de LVGL.

## 4. Modelo de execução (deep-sleep-cêntrico)

Não há `loop()` contínuo: **cada toque no botão é um boot saindo do deep sleep**.

```
[deep sleep] --botão--> boot → setup():
  1. lê motivo do wake (esp_sleep_get_wakeup_cause) e limpa IRQ do PMU
  2. init mínimo: PMU/rails (AXP2101) → display (LovyanGFX) → I2C → PCF8563 → BMA423
  3. CPU @ 80 MHz; rádio/GPS/haptics off; touch ON (só agora)
  4. needsNtpSync()? → liga WiFi → SNTP → grava PCF8563 → desliga WiFi (timeout 8s, best-effort)
  5. lê hora/bateria/passos; currentPage = CLOCK; desenha a página
loop():
  - lê touch; classifica gesto (swipe ↑/↓, double-tap); aplica ação; reinicia timer
  - atualiza segundos ~1×/s
  - se (millis - lastInteraction > 10s):
        setBrightness(0) → desliga rails de display e touch (BMA permanece) →
        configura wake = botão → esp_deep_sleep_start()
```

**Estado persistente (`RTC_DATA_ATTR`, sobrevive ao deep sleep):**
`bootCount`, `everSynced` (bool), `lastSyncYearDay` (int), `lastSyncOk` (bool). A página corrente **não** persiste — sempre reabre em CLOCK.

## 5. Componentes (unidades isoladas)

Cada unidade com um propósito, interface clara e testável:

- **`config.h`** — `WIFI_SSID`, `WIFI_PASS`, `TZ_STRING` (default `"<-03>3"`, America/Sao_Paulo sem DST), `NTP_SERVER` (`pool.ntp.org`), `SCREEN_TIMEOUT_MS` (10000), `AWAKE_BRIGHTNESS` (80), `PEDOMETER_ENABLED` (1).
- **`power.{h,cpp}`** — `powerInit()` (sequência AXP2101), `setBrightness(level)` (0 desliga rail), `setCpuLow()` (80 MHz), `peripheralsOff()` (rádio/GPS/haptics), `enterDeepSleepButtonWake()`.
- **`display.{h,cpp}`** — `displayInit()`, `renderClockPage(state)`, `renderSystemPage(state)`, `renderDatePage(state)`. Desenho MAINFRAME direto em LovyanGFX.
- **`timekeep.{h,cpp}`** — `rtcInit()`, `readClock(&out)`, `ntpSync()` (WiFi up→SNTP→set RTC→WiFi down). Função **pura**: `needsNtpSync(everSynced, lastSyncYearDay, currentYearDay)`.
- **`input.{h,cpp}`** — `touchInit()`, `pollGesture()` → `{NONE, SWIPE_UP, SWIPE_DOWN, DOUBLE_TAP}`. Função **pura**: `classifyGesture(dx, dy, dt_ms)`.
- **`pedometer.{h,cpp}`** — `pedoInit()` (BMA423 step-counter low-power, mantido no sleep), `getSteps()`.
- **`battery.h`** — função **pura** `batteryPercent(mV)` (4200→100%, 3300→0%).
- **`main.cpp`** — orquestra o fluxo de wake/render/sleep acima.

## 6. As três páginas (swipe ↑/↓ cicla 0→1→2→0)

- **0 · CLOCK**: header `WIFI:OK`/`--`/`SYNC...` (esq.) e `BAT 78%` (dir.); hora `21:42` vermelha grande + `:09` laranja; data `// TUE 2026-06-04`; barra `[||||||||····] PWR`. Glyph de carga se `isCharging()`.
- **1 · SYSTEM**: bateria % + mV, status de carga, temperatura do core, status/idade da última sync NTP.
- **2 · DATE**: dia da semana + data completa, **passos** (BMA423), uptime desde boot frio / `bootCount`.

## 7. Tratamento de erros

- **NTP falha / sem WiFi**: mantém a hora do PCF8563, marca sync como velha (ícone `--`), tenta de novo no próximo dia/wake. Timeout de conexão WiFi: 8 s, depois aborta e segue.
- **RTC sem hora válida** (boot frio, ano < 2020): força tentativa de NTP; sem WiFi, mostra `--:--` até sincronizar.
- **Falha de init (PMU/display/sensor)**: loga no serial; segue best-effort (display é o crítico — se falhar, reinicia uma vez).
- **Bateria crítica** (< ~3,3V): após mostrar aviso, deep sleep direto (evita brownout).

## 8. Resultado esperado de bateria

Domina o standby em deep sleep. Estimativa conservadora (bateria ~300 mAh):
- Standby deep sleep (AXP2101 + RTC + BMA step-counter): ~0,5–1 mA.
- ~20 olhadas/dia × 12 s × ~80 mA ≈ 5–6 mAh/dia.
- 1 sync NTP/dia × ~8 s × ~120 mA ≈ 0,3 mAh/dia.
- Total ≈ 30 mAh/dia → **~10 dias**; mesmo pessimista (2 mA standby) sobram **vários dias**. Meta de 20h amplamente superada. Risco: acertar a config de rails do AXP2101 para o deep sleep ficar baixo de fato (mitigado reaproveitando a sequência provada).

## 9. Build, gravação e medição

- `pio run -e twatch-s3 -t upload --upload-port COM5` (após reescrever `platformio.ini`).
- Log serial a cada wake: motivo do wake, tensão da bateria, resultado da sync, passos.
- **Validação de bateria**: medir queda de % em 24h **na bateria** (desconectado do USB). Opcional: medidor USB inline para confirmar corrente de standby.

## 10. Testes

- **Funções puras** (`batteryPercent`, `needsNtpSync`, `classifyGesture`, lógica de timeout) isoladas em arquivos compiláveis no host → env `native` do PlatformIO com asserts simples. Cobre a lógica sem hardware.
- **Comportamento de hardware** validado por logs serial + o teste de 24h.

## 11. Fora de escopo (YAGNI)

Múltiplas faces/temas, swipe ←/→ para brilho, notificações, áudio/microfone, rádio LoRa, GPS, BLE. Podem virar specs futuras.

## 12. Riscos e mitigações

| Risco | Mitigação |
|---|---|
| Config de rails AXP2101 errada → tela apagada ou standby alto | Reusar `beginPower()` provado; medir standby |
| Wake `ext1` no GPIO21 não disparar | GPIO21 é RTC-capable no S3; validar com log de wake cause |
| Step counter zerar no sleep | Não cortar o rail do BMA; confirmar persistência via log |
| Repo não compila hoje | `platformio.ini` reescrito + stack consistente verificada no primeiro build |
