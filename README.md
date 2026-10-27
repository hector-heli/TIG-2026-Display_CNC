# Display Scripts

Teclado num茅rico t谩ctil para ESP32 con pantalla TFT ST7796 de 3.5" (320x480) y
controlador t谩ctil XPT2046, basado en el ejemplo *Arduin-o-phone* de la librer铆a
TFT_eSPI.

## Funcionalidad

- Teclado de 15 teclas: `New`, `Del`, `Send`, `0-9`, `.` y `#`.
- Visor del n煤mero introducido (m谩ximo 12 caracteres).
- Comunicaci髇 con DLC32 por WiFi TCP (por defecto 192.168.4.1:23) o UART como fallback.
- Calibraci贸n t谩ctil almacenada en SPIFFS (`/TouchCalData2`) y ligada a la
  rotaci贸n de la pantalla.
- Dise帽o en horizontal 480x320 (`DISPLAY_ROTATION 1`), con el teclado a la
  izquierda y el visor/estado a la derecha.

## Hardware

| Se帽al TFT | GPIO ESP32 |
|-----------|------------|
| MISO (SDO) | 12 |
| MOSI (SDI) | 13 |
| SCLK | 14 |
| CS | 15 |
| DC (RS/AO) | 2 |
| RST | -1 (conectado a RST de la placa) |
| BL (backlight) | 27 |
| T_CS (touch) | 33 |

- Placa: ESP32-3248S035 (CYD 3.5"), ESP32 Dev Module, 4 MB de flash, PSRAM
  deshabilitada.
- Pantalla: ST7796, 320x480, orden de color BGR.
- Frecuencias SPI: 27 MHz pantalla, 20 MHz lectura, 2.5 MHz t谩ctil.

Si la pantalla no muestra imagen o lo hace parcialmente, comprueba que el
controlador sea ST7796; para variantes ILI9488 cambia el driver en
`config/User_Setup.h` a `ILI9488_DRIVER`.

## Requisitos

- Arduino IDE 2.x.
- Core ESP32 `3.3.10` (probado con esa versi贸n).
- Librer铆a [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) en
  `Documents/Arduino/libraries/TFT_eSPI`.

## Configuraci贸n de TFT_eSPI

La configuraci贸n de pines y driver no vive en este repositorio dentro de la
librer铆a, por lo que hay que instalarla una vez:

1. Haz una copia de seguridad de `Documents/Arduino/libraries/TFT_eSPI/User_Setup.h`.
2. Copia `config/User_Setup.h` sobre ese archivo.
3. Verifica que `TFT_eSPI/User_Setup_Select.h` tenga activa la l铆nea:

   ```cpp
   #include <User_Setup.h>
   ```

El archivo `config/User_Setup.h` es la fuente de verdad para el cableado y debe
actualizarse aqu铆 si cambia el hardware. Incluye el driver `ST7796_DRIVER`, la
resoluci贸n `320x480` y el orden de color `TFT_BGR`.

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

La configuraci贸n de VS Code (`arduino.json`) genera los binarios en `binary/`,
que est谩 excluido del control de versiones.

## Configuraci髇 WiFi/TCP\r\n\r\n1. En la pantalla de Ajustes del pendant, usar 'Modo: UART/WiFi' para cambiar entre modos (requiere reinicio tras guardar).\r\n2. Usar 'WiFi/DLC32' para alternar IP destino (192.168.4.1 / 192.168.1.100 / 192.168.1.50) y puerto (23/8080).\r\n3. Las credenciales WiFi se guardan en Preferences (namespace pendant, claves wifissid/wifipass). Por defecto DLC32 en AP suele usar IP 192.168.4.1 puerto 23.\r\n4. En modo WiFi se reintenta conexi髇 autom醫icamente. El diagn髎tico muestra estado WiFi/TCP/IP.\r\n\r\n## Uso\r\n\r\n- Al primer arranque, o si cambia la rotaci贸n, aparece la pantalla de
  calibraci贸n: toca las esquinas en el orden indicado.
- Para forzar una nueva calibraci贸n, pon `REPEAT_CAL true` en `Archivo.ino`,
  recompila, calibra y vuelve a ponerlo en `false`.
- Para cambiar la orientaci贸n, modifica `DISPLAY_ROTATION` (1 = horizontal con
  teclado a la izquierda, 3 = horizontal invertida); la calibraci贸n anterior se
  descarta autom谩ticamente.

## Estructura

```
Archivo/
  Archivo.ino        Sketch principal
  Free_Fonts.h       Cabecera auxiliar de fuentes (no usada por el sketch)
config/
  User_Setup.h       Configuraci贸n de TFT_eSPI para este hardware
.vscode/
  arduino.json       Configuraci贸n de compilaci贸n de Arduino para VS Code
README.md
```

## Soluci贸n de problemas

- Pantalla girada o espejada: cambia `DISPLAY_ROTATION` a `3`.
- Colores invertidos: prueba `TFT_INVERSION_ON` o `TFT_INVERSION_OFF` en
  `config/User_Setup.h`.
- Sin imagen o imagen parcial: verifica el controlador (ST7796 o ILI9488) en
  `config/User_Setup.h`.

## Notas

- La calibraci贸n se guarda como 10 bytes de datos m谩s 1 byte con la rotaci贸n.
- Los archivos de compilaci贸n (`binary/`, `.elf`, `.map`, `.bin`) no se versionan.
