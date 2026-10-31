# Display Scripts

Pendant táctil para ESP32 con pantalla TFT ST7796 de 3.5" (320x480), controlador
táctil XPT2046 y enlace con un DLC32 por WiFi/Telnet o UART.

## Funcionalidad

- Pantalla principal con DRO, estado GRBL, jog por ejes, pasos y avances.
- Pantalla de husillo con control de RPM, marcha/paro y overrides.
- Pantalla **Caja / Desbaste** para generar una cavidad rectangular con
  trayectorias raster y ejecutarla por streaming.
- Apagado automático del backlight tras 3 minutos sin uso y despertar por toque,
  error o alarma.
- LED RGB de estado: verde al ejecutar, ámbar en pausa, rojo en error/alarma,
  cian en homing, amarillo sin enlace y azul tenue en reposo.
- Vista isométrica proporcional de la caja con cotas `L`, `W`, `H`.
- Teclado numérico táctil para editar dimensiones y parámetros de corte.
- Comunicación con DLC32 por WiFi TCP (por defecto `192.168.4.1:23`) o UART.
- Calibración táctil almacenada en SPIFFS (`/TouchCalData2`) y ligada a la
  rotación de la pantalla.
- Diseño en horizontal 480x320 (`DISPLAY_ROTATION 1`).

## Hardware

| Señal TFT | GPIO ESP32 |
|-----------|------------|
| MISO (SDO) | 12 |
| MOSI (SDI) | 13 |
| SCLK | 14 |
| CS | 15 |
| DC (RS/AO) | 2 |
| RST | -1 (conectado a RST de la placa) |
| BL (backlight) | 27 |
| T_CS (touch) | 33 |
| LED RGB R / G / B | 4 / 16 / 17 (activo bajo) |

- Placa: ESP32-3248S035 (CYD 3.5"), ESP32 Dev Module, 4 MB de flash, PSRAM
  deshabilitada.
- Pantalla: ST7796, 320x480, orden de color BGR.
- Frecuencias SPI: 27 MHz pantalla, 20 MHz lectura, 2.5 MHz táctil.

Si la pantalla no muestra imagen o lo hace parcialmente, comprueba que el
controlador sea ST7796; para variantes ILI9488 cambia el driver en
`config/User_Setup.h` a `ILI9488_DRIVER`.

## Requisitos

- Arduino IDE 2.x.
- Core ESP32 `3.3.12` (probado con esa versión).
- Librería [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) en
  `Documents/Arduino/libraries/TFT_eSPI`.

## Configuración de TFT_eSPI

Instala TFT_eSPI en `Documents/Arduino/libraries/TFT_eSPI`. El archivo
`Archivo/tft_setup.h` carga automáticamente `config/User_Setup.h` durante la
compilación, sin modificar los archivos de la librería. Este archivo del
repositorio es la fuente de verdad para el cableado y el driver: incluye
`ST7796_DRIVER`, la resolución `320x480`, el orden de color `TFT_BGR` y la
configuración del controlador táctil.

## Compilar y subir

1. Abre `Archivo/Archivo.ino` en Arduino IDE.
2. Selecciona la placa **ESP32 Dev Module** con:
   - CPU Frequency: 240 MHz
   - Flash Size: 4 MB
   - Partition Scheme: Default 4MB with spiffs
   - PSRAM: Disabled
   - Upload Speed: 921600
3. Selecciona el puerto serie de la placa (por ejemplo `COM6`).
4. Compila y sube.

La configuración de VS Code (`arduino.json`) genera los binarios en `binary/`,
que está excluido del control de versiones.

## Configuración WiFi/TCP

1. En la pantalla de Ajustes del pendant, usa `Modo: UART/WiFi` para cambiar
   entre modos (requiere reinicio tras guardar).
2. Usa `WiFi/DLC32` para alternar IP destino (`192.168.4.1` / `192.168.1.100`
   / `192.168.1.50`) y puerto (`23` / `8080`).
3. Las credenciales WiFi se guardan en Preferences (namespace `pendant`, claves
   `wifissid` / `wifipass`). El DLC32 en modo AP suele usar `192.168.4.1:23`;
   `8080` queda como alternativa. El puerto `80` es HTTP y no entrega GRBL.
4. En modo WiFi se reintenta la conexión automáticamente. El diagnóstico muestra
   estado WiFi/TCP/IP. Si TCP sigue abierto pero no llega estado GRBL durante
   `TCP_STALE_MS` (3 s), se cierra y se reconecta para liberar sesiones Telnet
   colgadas del DLC32. Tras 3 intentos sin estado GRBL se espera
   `TCP_RETRY_COOLDOWN_MS` (30 s) para no saturar el servidor Telnet.
5. El DLC32 acepta **un solo cliente Telnet**. Si conecta y cierra de inmediato
   sin entregar estado (`TCP connect ... OK` seguido de
   `TCP disconnected no-status`), el slot quedó ocupado por una sesión anterior:
   reinicia el DLC32 antes de reconectar.

## Caja / Desbaste

La cavidad se define con origen en el **centro X/Y** y **Z=0 en la cara
superior** de la pieza. `L`, `W` y `H` son las dimensiones finales de la
cavidad; las esquinas interiores quedan con el radio de la fresa.

Parámetros por defecto:

| Parámetro | Valor |
|-----------|-------|
| Largo X | 40 mm |
| Ancho Y | 30 mm |
| Altura Z | 10 mm |
| Fresa | Ø6 mm |
| Paso lateral | 40 % |
| Avance | 400 mm/min |
| RPM | 900 |
| Z seguro | 10 mm |
| Bajada por pasada | 1 mm |
| Avance de penetración | 200 mm/min |

Flujo de uso:

1. En `Caja`, toca un valor de `Dimensiones` o `Parámetros` y edítalo con el
   teclado. `OK` guarda en NVS; `Cancel` descarta.
2. Toca `Generar` para crear `/box.nc` en SPIFFS. El raster recorre Y con
   zigzag en X y baja en capas de Z.
3. Toca `Ejecutar` dos veces para confirmar. El pendant envía una línea y espera
   `ok` antes de enviar la siguiente. Mientras GRBL reporta `Run`, `Jog` o
   `Home` no se aplica el timeout corto de comandos; al quedar quieto se usan
   5 segundos para recibir el `ok`.
4. `Pausa` envía `!`; `Reanudar` envía `~`; `Detener` pide confirmación y envía
   `!` seguido de `Ctrl-X` (`0x18`).
5. Si se pierde el enlace, hay timeout o GRBL reporta error/alarma, el trabajo
   se aborta y aparece un aviso bloqueante con `M5 HUSILLO`, `RESET GRBL` y
   `LISTO`. No se reanuda automáticamente.
6. El aviso mantiene el husillo como prioridad: `M5` intenta detenerlo, `RESET`
   envía `Ctrl-X` y `LISTO` cierra el aviso después de verificar la máquina.

El streaming no es fail-safe: si cae WiFi, el DLC32 puede terminar las líneas
que ya tenía en su buffer. Mantén el área de trabajo despejada y usa `Detener`
o el paro físico si es necesario.

## Pantalla y LED RGB

- El backlight se apaga tras `BACKLIGHT_TIMEOUT_MS` (3 minutos) sin toques.
- El primer toque con la pantalla apagada solo la enciende; no activa ningún
  botón.
- Un error, alarma o aviso de aborto enciende la pantalla automáticamente.
- La pantalla también puede apagarse durante un trabajo; el LED sigue mostrando
  el estado.
- El LED RGB no se apaga con el backlight.
- En la CYD 3.5" el LED RGB es activo bajo (`RGB_LED_ACTIVE_LOW 1`). Si tu
  variante usara lógica activa alta, cambia ese define a `0` en `Config.h`.

| Estado | LED |
|---|---|
| Alarma | Rojo intermitente |
| Error de trabajo / aviso de aborto | Rojo fijo |
| Error reciente (6 s) | Rojo tenue |
| Homing | Cian |
| Sin enlace | Amarillo intermitente |
| Pausa / GRBL Hold | Ámbar |
| Caja o trabajo ejecutándose | Verde |
| Reposo con enlace OK | Azul tenue |

`M4` requiere `$32=0` en el DLC32 y que el driver del husillo tenga conectada
la señal de dirección (`DIR`). Si el comando recibe `ok` pero el husillo no
invierte, revisa el cableado o el driver.

## Uso

- Al primer arranque, o si cambia la rotación, aparece la pantalla de
  calibración: toca las esquinas en el orden indicado.
- Para forzar una nueva calibración, pon `REPEAT_CAL true` en `Archivo.ino`,
  recompila, calibra y vuelve a ponerlo en `false`.
- Para cambiar la orientación, modifica `DISPLAY_ROTATION` (1 = horizontal con
  teclado a la izquierda, 3 = horizontal invertida); la calibración anterior se
  descarta automáticamente.

## Estructura

```
Archivo/
  Archivo.ino        Sketch principal y loop
  Config.h           Estados, límites y valores por defecto
  Settings.ino       Carga/guardado en NVS
  Link.ino           WiFi/TCP y UART
  Grbl.ino           Parseo de estado GRBL y envío de comandos
  Box.ino            Generación de /box.nc y streaming del trabajo
  Backlight.ino      Apagado automático y despertar del backlight
  UI.ino             Pantallas, toque y teclado numérico
  tft_setup.h        Conecta la configuración TFT_eSPI del repositorio
  Free_Fonts.h       Cabecera auxiliar de fuentes (no usada por el sketch)
config/
  User_Setup.h       Configuración de TFT_eSPI para este hardware
GCode/
  rutinaDesbaste.nc  Referencia del patrón original de desbaste
.vscode/
  arduino.json       Configuración de compilación de Arduino para VS Code
README.md
```

## Solución de problemas

- Pantalla girada o espejada: cambia `DISPLAY_ROTATION` a `3`.
- Colores invertidos: prueba `TFT_INVERSION_ON` o `TFT_INVERSION_OFF` en
  `config/User_Setup.h`.
- Sin imagen o imagen parcial: verifica el controlador (ST7796 o ILI9488) en
  `config/User_Setup.h`.
- WiFi conectado pero sin respuesta GRBL: confirma `192.168.4.1:23`, cierra otras
  sesiones Telnet y reinicia el DLC32 si el único slot quedó ocupado. Si el log
  muestra `TCP connect ... OK` y cierra sin estado, el slot sigue ocupado; el
  pendant espera 30 s entre tandas de reintentos.
- `Generar` falla por tamaño: aumenta el diámetro de fresa o el paso lateral, o
  reduce la altura. El límite es `BOX_MAX_LINES` (20000 líneas).
- El trabajo no reanuda tras una pausa: revisa el enlace y el estado GRBL; si
  hubo error o alarma, el trabajo se aborta y debe generarse/ejecutarse de nuevo.
- La pantalla queda negra: toca una vez para encenderla. Si no responde y el
  LED está amarillo intermitente, revisa el enlace con el DLC32.
- El LED no coincide con el estado: confirma la polaridad del LED RGB. La CYD
  3.5" usa lógica activa baja (`RGB_LED_ACTIVE_LOW 1` en `Config.h`).
- Pérdida de enlace durante un trabajo: el monitor serie registra
  `TCP disconnected` o `Link down` con la antigüedad del último estado. Revisa
  WiFi, fuente de alimentación, interferencias del husillo y que no haya otra
  sesión Telnet abierta en el DLC32. Al arrancar se imprime `reset=...`
  (`esp_reset_reason`); un valor de brownout indica alimentación inestable.
- La placa se calienta: cierto calor por WiFi, backlight y reguladores es
  normal. Si es excesivo, usa una fuente 5 V/1 A estable y confirma que el LED
  RGB no quede a máxima intensidad por polaridad invertida.

## Notas

- La calibración se guarda como 10 bytes de datos más 1 byte con la rotación.
- Los archivos de compilación (`binary/`, `.elf`, `.map`, `.bin`) no se versionan.
