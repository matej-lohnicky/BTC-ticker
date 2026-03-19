#include <Arduino.h>
#include <modules/screenshot.h>

namespace
{

constexpr uint8_t PROTOCOL_VERSION = 1;
constexpr uint8_t FRAME_MAGIC[4] = {'S', 'H', 'O', 'T'};

void write_header(ScreenshotScreen screen, uint16_t width, uint16_t height)
{
    const uint32_t payloadBytes =
        static_cast<uint32_t>(width) * static_cast<uint32_t>(height) * 2UL;
    uint8_t header[14] = {0};
    header[0] = FRAME_MAGIC[0];
    header[1] = FRAME_MAGIC[1];
    header[2] = FRAME_MAGIC[2];
    header[3] = FRAME_MAGIC[3];
    header[4] = PROTOCOL_VERSION;
    header[5] = static_cast<uint8_t>(screen);
    header[6] = static_cast<uint8_t>(width & 0xFF);
    header[7] = static_cast<uint8_t>((width >> 8) & 0xFF);
    header[8] = static_cast<uint8_t>(height & 0xFF);
    header[9] = static_cast<uint8_t>((height >> 8) & 0xFF);
    header[10] = static_cast<uint8_t>(payloadBytes & 0xFF);
    header[11] = static_cast<uint8_t>((payloadBytes >> 8) & 0xFF);
    header[12] = static_cast<uint8_t>((payloadBytes >> 16) & 0xFF);
    header[13] = static_cast<uint8_t>((payloadBytes >> 24) & 0xFF);

    Serial.write(header, sizeof(header));
}

}  // namespace

void screenshot_send_rgb565_frame(ScreenshotScreen screen, uint16_t width, uint16_t height,
                                  const uint16_t* buffer)
{
    if (buffer == nullptr || width == 0 || height == 0 || width > 320)
    {
        return;
    }

    write_header(screen, width, height);
    Serial.write(reinterpret_cast<const uint8_t*>(buffer),
                 static_cast<size_t>(width) * static_cast<size_t>(height) * 2U);
    Serial.flush();
}

void screenshot_send_complete()
{
    write_header(ScreenshotScreen::Complete, 0, 0);
    Serial.flush();
}
