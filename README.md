# Display Scripts

Teclado numérico táctil para ESP32 con pantalla TFT ST7796 de 3.5" (320x480) y
controlador táctil XPT2046, basado en el ejemplo *Arduin-o-phone* de la librería
TFT_eSPI.

## Funcionalidad

- Teclado de 15 teclas: `New`, `Del`, `Send`, `0-9`, `.` y `#`.
- Visor del número introducido (máximo 12 caracteres).
- `Send` envía el número por el puerto serie a 115200 baudios.
- Calibración táctil almacenada en SPIFFS (`/TouchCalData2`) y ligada a la
  rotación de la pantalla.
- Diseño en horizontal 480x320 (`DISPLAY_ROTATION 1`), con el teclado a la
  izquierda y el visor/estado a la derecha.

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

- Placa: ESP32-3248S035 (CYD 3.5"), ESP32 Dev Module, 4 MB de flash, PSRAM
  deshabilitada.
- Pantalla: ST7796, 320x480, orden de color BGR.
- Frecuencias SPI: 27 MHz pantalla, 20 MHz lectura, 2.5 MHz táctil.

Si la pantalla no muestra imagen o lo hace parcialmente, comprueba que el
controlador sea ST7796; para variantes ILI9488 cambia el driver en
`config/User_Setup.h` a `ILI9488_DRIVER`.

## Requisitos

- Arduino IDE 2.x.
- Core ESP32 `3.3.10` (probado con esa versión).
- Librería [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) en
  `Documents/Arduino/libraries/TFT_eSPI`.

## Configuración de TFT_eSPI

La configuración de pines y driver no vive en este repositorio dentro de la
librería, por lo que hay que instalarla una vez:

1. Haz una copia de seguridad de `Documents/Arduino/libraries/TFT_eSPI/User_Setup.h`.
2. Copia `config/User_Setup.h` sobre ese archivo.
3. Verifica que `TFT_eSPI/User_Setup_Select.h` tenga activa la línea:

   ```cpp
   #include <User_Setup.h>
   ```

El archivo `config/User_Setup.h` es la fuente de verdad para el cableado y debe
actualizarse aquí si cambia el hardware. Incluye el driver `ST7796_DRIVER`, la
resolución `320x480` y el orden de color `TFT_BGR`.

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
  Archivo.ino        Sketch principal
  Free_Fonts.h       Cabecera auxiliar de fuentes (no usada por el sketch)
config/
  User_Setup.h       Configuración de TFT_eSPI para este hardware
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

## Notas

- La calibración se guarda como 10 bytes de datos más 1 byte con la rotación.
- Los archivos de compilación (`binary/`, `.elf`, `.map`, `.bin`) no se versionan.
